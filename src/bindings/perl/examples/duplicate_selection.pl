my $frame = abi::XAP_Frame::getLastFocussed;
my $view = $frame->getCurrentView;
$text = $view->getSelectionText;
$view->write($text);
1;
