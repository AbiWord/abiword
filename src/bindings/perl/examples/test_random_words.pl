open(TEXT, "/home/cuenca/cvs/abiword/abi/COPYING") or die "eccck.  Do you have a /home/cuenca?? :-) : $!";

my $frame = abi::XAP_Frame::getLastFocussed;
my $view = $frame->getCurrentView;
my @words;
# most probable choices are the most repeated ones
my @seps = (" ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
	    " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
	    "\t", "\n", "\n\n", "");
	    
while (<TEXT>) {
    push(@words, split(/\s+/));
}

my $init_time = time;
my $def_style = 1;
my $i;

for ($i = 0; $i < 3001; ++$i) {
    if (($i % 100) == 0) {
	my $current_time = time;
	my $delta = $current_time - $init_time;
	printf("$i $delta\n");
	$init_time = $current_time;
    }

    $def_style = &maybeChangeStyle($view, $def_style);
    my $word = &pickoneof(@words);
    $view->write($word) unless ($word eq "");
    my $tmp = &pickoneof(@seps);
    $view->write($tmp) unless ($tmp eq "");
}

close TEXT;
1;

sub pickoneof
{
    return $_[int(rand(scalar(@_) -1))];
}

sub maybeChangeStyle
{
    my $view = shift;
    my $defst = shift;
    my @ftts = ({"font-weight" => "bold"},
  		{"font-weight" => "bold"},
  		{"font-weight" => "bold"},
  		{"font-style" => "italic"},
  		{"font-style" => "italic"},
  		{"font-weight" => "bold",
  		 "font-size" => "24pt"},
  		{"font-family" => "Courier New",
  		 "font-size" => "11pt"},
  		{"font-style" => "normal",
  		 "text-decoration" => "underline"});
    if ($defst == 1) {
  	if (rand(100) > 95) {
	    my $char_prop = &pickoneof(@ftts);
  	    $view->setCharFormat(%{$char_prop});
  	    return 0;
  	}
    }
    else {
	my $tmp = rand(100);
	if ($tmp > 75) {
	    $view->setCharFormat("font-weight" => "normal",
				 "font-style" => "normal",
				 "font-family" => "Times New Roman",
				 "font-size" => "12pt",
				 "text-decoration" => "normal");
	    return 1;
	}
	elsif ($tmp > 70) {
	    my $char_prop = &pickoneof(@ftts);
	    $view->setCharFormat(%{$char_prop});
	    return 0;
	}
    }

    return 0;
}
