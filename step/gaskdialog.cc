#include "gaskdialog.h"

void GasKDialog::slotButtonClicked(int button) {
   if(button == KDialog::Ok) {
       if(_handler->createGasParticlesApply()) accept();
   } else {
       KDialog::slotButtonClicked(button);
   }
}
