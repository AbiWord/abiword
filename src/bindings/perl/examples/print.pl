sub PrintNonInteractive {
    my $frame = AbiWord::XAP_Frame::getLastFocussed;
    my $view = $frame->getCurrentView;
    $view->print;
}

sub PrintInteractive {
    my $frame = AbiWord::XAP_Frame::getLastFocussed;
    my $view = $frame->getCurrentView;
    $view->showPrintDialog;
}

AbiWord::XAP_Frame::register("PrintNonInteractive", "&File/PerlPrint", "Another item for print the document", false);
AbiWord::XAP_Frame::register("PrintInteractive", "&File/PerlPrint Interactive", "And another one", true);

1;
