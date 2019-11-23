#!/bin/env python3

import os
import re
import sys
import git


def get_start_rev(major, minor, micro):
    if int(micro) == 0:
        raise Exception("Wrong version {}.{}.{}".format(major, minor, micro))
    return "release-{}.{}.{}".format(major, minor, int(micro) - 1)

def get_end_rev(major, minor, micro):
    return "release-{}.{}.{}".format(major, minor, micro)

# Remove the "cherry-picked from" lines
def strip_cherry_pick(msg):
    regexp = re.compile(r"^\(?cherry[ \-]pick(ed)? from.*$", flags=re.I|re.M)
    msg = regexp.sub("", msg)
    return msg

# Strip the backport mentions
def strip_backports(msg):
    regexp = re.compile(r"\(?(Please backport|Can be backported|Feel free to backport)\.?\)?", flags=re.I)
    msg = regexp.sub("", msg)
    return msg

# Strip the svn ids. Shouldn't be needed past 3.0.3
def strip_svn_id(msg):
    regexp = re.compile(r"^git-svn-id:.*$", flags=re.M)
    msg = regexp.sub("", msg)
    return msg

# Strip signoff mentions
def strip_signoff(msg):
    regexp = re.compile(r"^Signed-off-by:.*$", flags=re.M)
    msg = regexp.sub("", msg)
    return msg

# Linkify bugzilla links
def add_bugzilla_links(msg):
    m = re.match(r"(.*?)((Bug\s|Fix\s)?(\d\d\d+))(.*)", msg)
    if m:
        msg = m.group(1) + "<a href=\"http://bugzilla.abisource.com/show_bug.cgi?id=" + m.group(4) + "\">" + m.group(2) + "</a>" + m.group(5)

    return msg


def classify_topic(paths, msg):
    topic = "Core"
    sub_topic = "-"

    for path in paths:
        if re.match(r".*\/opendocument\/.*", path):
            topic = 'Import/Export'
            sub_topic = 'OpenDocument'
            break

        if re.match(r".*\/openxml\/.*", path):
            topic = 'Import/Export'
            sub_topic = 'Office Open XML'
            break

        if re.match(r".*\/goffice\/.*", path):
            topic = 'Plugins'
            sub_topic = 'GNOME Office'
            break

        if re.match(".*\/collab\/.*", path):
            topic = 'Plugins';
            sub_topic = 'Collaboration';
            break;

        if re.match(r".*\/cocoa\/.*", path):
            topic = 'Interface';
            sub_topic = 'Mac OSX';
            break;

        if re.match(".*MSWrite.*", path):
            topic = 'Plugins';
            sub_topic = 'MS Write';
            break;

    # commit message based classification
    if re.match(r".*translation.*", msg):
        topic = 'Internationalization'

    regexp = re.compile(r".*buildfix.*", "i")
    if regexp.match(msg):
        topic = "Development"

    regexp = re.compile(r".*bump version.*", "i")
    if regexp.match(msg):
        topic = 'Development'

    return (topic, sub_topic)


def print_preamble(major, minor, micro):
    print("<?")
    print("include(\"../format.inc\");")
    print("printHeader(\"AbiWord v{}.{}.{} ChangeLog\");".format(major, minor, micro))
    print("?>")
    print("<h1>AbiWord v{}.{}.{} Changelog</h1>".format(major, minor, micro))

def print_postamble():
    print("<?\nprintFooter();\n?>")

# get_commits from repository from_rev..to_rev
# return the list of commits
def get_commits(repo, from_rev, to_rev):
    # if we can't resolve to_rev we'll consider the tip to the branch
    try:
        t = repo.commit(to_rev)
    except:
        to_rev = ""

    try:
        commits = list(repo.iter_commits(rev="{}..{}".format(from_rev, to_rev)))
    except:
        commits = []
    return commits

# Print the changelog for the commits list
def print_changelog(repo, commits):
    print("<ul>")
    for commit in commits :
        c = repo.commit(commit)
        msg = strip_svn_id(c.message)
        msg = strip_backports(msg)
        msg = strip_cherry_pick(msg)
        msg = strip_signoff(msg)
        msg = add_bugzilla_links(msg)
        msg = msg.strip()
        print("  <li>{} ({})</li>".format(msg, c.author))
    print("</ul>")


try:
    # get the version from argument one
    if len(sys.argv) < 2:
        raise Exception("Please pass the version you want to generate the changelog for")

    argv1 = sys.argv[1]
    (major, minor, micro) = argv1.split(".", maxsplit=3);

    start = get_start_rev(major, minor, micro)
    end = get_end_rev(major, minor, micro)

    repo = git.Repo.init(os.path.join("."))

    commits = get_commits(repo, start, end)
    if len(commits) == 0:
        raise Exception("No commit founds.")

    print_preamble(major, minor, micro)
    print_changelog(repo, commits)
    print_postamble()

except:
    raise
