my $true = 1;
my $false = 0;

my $frame = abi::XAP_Frame::getLastFocussed;
my $view = $frame->getCurrentView;
$view->findNext("TiTi", false);
1;
