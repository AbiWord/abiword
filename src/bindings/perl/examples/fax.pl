# test script for abiword
use strict;
use Gnome;
use Gtk;

my $false = 0;
my $true = 1;

Gnome->init("abiword", "0.7.14");

my $w = new Gnome::Dialog("Fax Document Composer", "Button_Ok", "Button_Cancel");
my $table = new Gtk::Table(3, 3, $false);
my $label = new Gtk::Label("Fax Number:");
my $fax_number = new Gtk::Entry;
my $company_name = new Gtk::Entry;
my $logo = new Gnome::PixmapEntry(0, "Select a logo", $true);
my $frame = new Gtk::Frame("Preview (FIXME!)");

$table->attach_defaults($label, 0, 1, 0, 1);
$label = new Gtk::Label("Company Name");
$table->attach_defaults($label, 0, 1, 1, 2);
$label = new Gtk::Label("Company Logo");
$table->attach_defaults($label, 0, 1, 2, 3);

$table->attach_defaults($fax_number, 1, 2, 0, 1);
$table->attach_defaults($company_name, 1, 2, 1, 2);
$table->attach_defaults($logo, 1, 2, 2, 3);

$table->attach_defaults($frame, 2, 3, 0, 3);

$table->set_row_spacings(2);
$table->set_col_spacings(8);

# we add the table to the dialog...
$w->vbox->add($table);

$w->signal_connect('clicked', \&clicked_cb, $fax_number, $company_name, $logo);
$w->show_all;

sub clicked_cb {
    my ($dialog, $fax_number, $company_name, $logo) = @_;
    my $button_nb = 1;

    print "$button_nb\n";
    if ($button_nb == 1) {
	my $frame = abi::XAP_Frame::getLastFocussed;
	my $view = $frame->getCurrentView;

	$view->setCharFormat("font-weight" => "bold", "font-size" => "24pt");
	$view->write("FAX Document.\t".$fax_number->get_text."\n\n");
	$view->setCharFormat("font-size" => "12pt");
	$view->write("From: ");
	$view->setCharFormat("font-weight" => "normal");
	$view->write("TODO (".$company_name->get_text.")\n");
	$view->setCharFormat("font-weight" => "bold");
	$view->write("To: ");
	$view->setCharFormat("font-weight" => "normal");
	$view->write("TODO\n");
    }
}

# return a true value
1;
