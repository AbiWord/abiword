sub toto {
    my $frame = AbiWord::XAP_Frame::getLastFocussed;
    my $view = $frame->getCurrentView;
    $view->write("Body 1\n");
    $view->editHeader;
    $view->write("Header");
    $view->editFooter;
    $view->write("Footer");
    $view->editBody;
    $view->write("Body 2\n");
}

AbiWord::XAP_Frame::register("toto", "&Edit/Toto", "Description toto", false);

1;
