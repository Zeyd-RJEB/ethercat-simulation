//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __ETHERCAT_ETHERCATMACMASTER_H_
#define __ETHERCAT_ETHERCATMACMASTER_H_

#include <omnetpp.h>
#include "EthernetFrame_m.h"
#include "../applicationlayer/EtherCatFrame_m.h"

/**
 * TODO - Generated class
 */
class EtherCatMACMaster : public cSimpleModule
{
  private:
      simtime_t delay;
      int type1;
      int type2;
      int type3;
      int type4;
      int type5;
      int type6;
      int type7;
      int byteReturn;
      int valueData;
  public:
    EtherCatMACMaster();
    virtual ~EtherCatMACMaster();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();


};

#endif
