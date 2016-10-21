<?php

define('REPOROOT', 'http://svn.abisource.com');
define('TAGROOT', REPOROOT . '/abiword/tags');

function help() {
	die('Usage: php ' . $_SERVER['argv'][0] . ' <x.y.z>' . "\n");
}

function get_tag_url($major, $minor, $micro) {
	return TAGROOT . "/release-$major.$minor.$micro";
}

function get_tag_copy_rev($tag_url) {
	return trim(`svn log --stop-on-copy --verbose --xml $tag_url | grep copyfrom-rev | cut -d "\"" -f 2`);
}

function get_changelog($from_rev, $to_rev, $url) {
	$xml = simplexml_load_string(`svn log -r$from_rev:$to_rev --xml --verbose $url`);
	if ($xml === FALSE)
		die("Error retrieving SVN log for range $from_rev:$to_rev from $url\n");

	return $xml;
}

function print_preamble($major, $minor, $micro) {
	print '<?
    include("../format.inc");
    printHeader("AbiWord v' . "$major.$minor.$micro" . ' ChangeLog");
?>

<h1>AbiWord v' . "$major.$minor.$micro" . ' Changelog</h1>
';
}

function print_postamble() {
	print '<?
    printFooter();
?>';
}

function is_backport_blocking($msg) {
	// check if this is an svnmerge.py blocked merge
	return preg_match('/^Blocked revision.*\.\.\.\.\.\.\.\.(.*)\.\.\.\.\.\.\.\..*$/ms', $msg);
}

function detect_embedded_commit($msg) {
	// check if this is an svnmerge.py recorded merge
	if (!preg_match('/^Recorded merge of.*\.\.\.\.\.\.\.\.(.*)\.\.\.\.\.\.\.\..*$/ms', $msg, $matches))
		return FALSE;

	$lines = explode("\n", trim($matches[1]));

	if (!preg_match('/.*\|(.*)\|.*\|.*$/', $lines[0], $matches))
		return FALSE;

	$author = trim($matches[1]);
	$msg = trim(implode(' ', array_slice($lines, 1)));

	return array($author, $msg);
}

function human_readable_author($author) {
	$table = array(
		'asfrent' => 'Andrei Sfrent',
		'cjl' => 'Chris Leonard',
		'dom' => 'Dominic Lachowicz',
		'hub' => 'Hubert Figui&egrave;re',
		'ib' => 'Ingo Br&uuml;ckl',
		'jbrefort' => 'Jean Br&eacute;fort',
		'monkeyiq' => 'Ben Martin',
		'msevior' => 'Martin Sevior',
		'pradeeban' => 'Kathiravelu Pradeeban',
		'slaroche' => 'Simon Larochelle',
		'strbafridrich' => 'Fridrich Strba',
		'uwog' => 'Marc Maurer',
		'volodymyr' => 'Volodymyr Rudyj',
		);
	if (array_key_exists($author, $table))
		return $table[$author];
	return $author;
}

function strip_backports($msg) {
	if (preg_match('/(.*)\(?(Please backport|Can be backported|Feel free to backport)\.?\)?(.*)/i', $msg, $matches))
		$msg = $matches[1] . $matches[3];

	return $msg;
}

function add_bugzilla_links($msg) {
	if (preg_match('/(.*?)((Bug\s|Fix\s)?(\d\d\d*))(.*)/', $msg, $matches)) {
		$msg = $matches[1] . '<a href="http://bugzilla.abisource.com/show_bug.cgi?id=' . $matches[4] . '">' . $matches[2] . '</a>' . $matches[5];
	}
	return $msg;
}

function classify_topic($paths, $msg) {
	$topic = 'Core';
	$sub_topic = '-';

	// path based classification
	foreach ($paths as $path) {
		// Feel free to expand this list to reduce the manual work

		if (preg_match('/.*\/po\/.*\.po/', $path)) {
			$topic = 'Internationalization';
			break;
		}

		if (preg_match('/.*\/opendocument\/.*/', $path)) {
			$topic = 'Import/Export';
			$sub_topic = 'OpenDocument';
			break;
		}

		if (preg_match('/.*\/openxml\/.*/', $path)) {
			$topic = 'Import/Export';
			$sub_topic = 'Office Open XML';
			break;
		}

		if (preg_match('/.*\/goffice\/.*/', $path)) {
			$topic = 'Plugins';
			$sub_topic = 'GNOME Office';
			break;
		}

		if (preg_match('/.*\/collab\/.*/', $path)) {
			$topic = 'Plugins';
			$sub_topic = 'Collaboration';
			break;
		}

		if (preg_match('/.*\/cocoa\/.*/', $path)) {
			$topic = 'Interface';
			$sub_topic = 'Mac OSX';
			break;
		}

		if (preg_match('/.*MSWrite.*/', $path)) {
			$topic = 'Plugins';
			$sub_topic = 'MS Write';
			break;
		}
	}

	// commit message based classification
	if (preg_match('/.*translation.*/', $msg)) {
		$topic = 'Internationalization';
	}

	if (preg_match('/.*buildfix.*/i', $msg)) {
		$topic = 'Development';
	}

	if (preg_match('/.*bump version.*/i', $msg)) {
		$topic = 'Development';
	}

	return array($topic, $sub_topic);
}

function parse_cherry_pick($msg) {
	$re = '/(Backported|Cherry[- ]pick(ed)?) from trunk@[0-9]*[.]*[0-9]*\.?( [Bb]y ([a-z]*)\.?)?/';
	if (preg_match($re, $msg, $matches) == 1) {
		$msg = preg_replace($re, '', $msg);
		if (count($matches) == 5) {
			return array($msg, $matches[4]);
		}
		return array($msg);
	}
	return FALSE;
}

function parse_logentries($xml) {
	$result = array();
	foreach ($xml->logentry as $logentry) {
		// get the author
		$author = trim((string)$logentry->author);

		// get the commit message if this commit was not a blocking of
		// blocking (and strip out all the svnmerge.py output if present)
		$msg = trim((string)$logentry->msg);
		if (is_backport_blocking($msg))
			continue;
		$commit = detect_embedded_commit($msg);
		if ($commit !== FALSE) {
			list($author, $msg) = $commit;
		}

		// get the affected paths
		$paths = array();
		foreach ($logentry->paths->path as $path) {
			$paths[] = (string)$path;
		}

	        $cherrypick = parse_cherry_pick($msg);
		if ($cherrypick != FALSE) {
			$msg = $cherrypick[0];
			if (count($cherrypick) > 1) {
				$author = $cherrypick[1];
			}
		}

		// convert the svn usernames into real-life names
		$author = human_readable_author($author);

		// strip double spaces from commit messages
		$msg = preg_replace('/\s\s+/', ' ', $msg);

		// strip the common "Please backport" and friends from commit messages
		$msg = strip_backports($msg);

		// add convenient links to bugzilla for bug numbers
		$msg = add_bugzilla_links($msg);

		// figure out which topic this commit belongs to
		list($topic, $sub_topic) = classify_topic($paths, $msg);

		$result[$topic][$sub_topic][] = array(trim($msg), $author);
	}
	return $result;
}

function print_changelog_section($logentries) {
	print "<ul>\n";
	foreach ($logentries as $logentry) {
		list($msg, $author) = $logentry;
		print "  <li>$msg ($author)</li>\n";
	}
	print "</ul>\n";
}

function print_changelog($xml) {
	$topics = parse_logentries($xml);
	foreach ($topics as $topic => $sub_topics) {
		print "<h2>$topic</h2>\n";

		// print items without a subtopic first...
		if (isset($sub_topics['-']))
			print_changelog_section($sub_topics['-']);

		// ... and then print all the subtopics
		foreach (array_diff_key($sub_topics, array('-' => '')) as $sub_topic => $logentries) {
			print "<h3>$sub_topic</h3>\n";
			print_changelog_section($logentries);
		}
	}
}

function main() {
	if (sizeof($_SERVER['argv']) != 2)
		help();

	// get the version number
	list($major, $minor, $micro) = explode('.', $_SERVER['argv'][1]);
	if (!is_numeric($major) || !is_numeric($minor) || !is_numeric($micro))
		help();

	if ($micro == 0)
		die("Sorry, you'll have to do x.y.0 releases by hand :)\n");


	// get the revision range between the current and previous tag
	$tag_url = get_tag_url($major, $minor, $micro);
	$prev_tag_url = get_tag_url($major, $minor, $micro - 1);
	
	$tag_copy_rev = get_tag_copy_rev($tag_url);
	$prev_tag_copy_rev = get_tag_copy_rev($prev_tag_url) + 1;

	// retrieve the changelog for this release
	$xml = get_changelog($prev_tag_copy_rev, $tag_copy_rev, $tag_url);

	// process the changelog and output a nicely formatted piece of HTML
	print_preamble($major, $minor, $micro);
	print_changelog($xml);
	print_postamble();
}

main();

?>
