my $frame = abi::XAP_Frame::getLastFocussed;
my $view = $frame->getCurrentView;
$text = $view->getSelectionText;
# I need a "clearSelection", or something...
$view->moveCursorRel("line", 1);
$view->write($text);
1;
