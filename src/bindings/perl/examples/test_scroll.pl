# open an empty document 
$frame = abi::XAP_Frame::getLastFocussed;
$view = $frame->getCurrentView;
$view->write("1\n");
$view->write("5\n");
$view->moveCursorRel("line", -1);
$view->write("3\n");
$view->moveCursorRel("line", -1);
$view->write("2\n");
$view->moveCursorRel("line", 1);
$view->write("4\n");

1;
