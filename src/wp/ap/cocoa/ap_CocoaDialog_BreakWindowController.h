/* AP_Dialog_BreakWindowController */

#import <Cocoa/Cocoa.h>

@interface AP_Dialog_BreakWindowController : NSWindowController
{
    IBOutlet NSMatrix *m_insertRadioBtn;
    IBOutlet NSMatrix *m_sectionRadionBtn;
}
- (IBAction)cancelPressed:(id)sender;
- (IBAction)okPressed:(id)sender;
@end
