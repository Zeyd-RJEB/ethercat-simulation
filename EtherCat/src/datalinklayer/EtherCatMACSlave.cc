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

#include "EtherCatMACSlave.h"
#include "EthernetFrame_m.h"

EtherCatMACSlave::EtherCatMACSlave()
{
}

EtherCatMACSlave::~EtherCatMACSlave()
{
}

Define_Module(EtherCatMACSlave);

void EtherCatMACSlave::initialize()
{
    delay=par("delaySlave");
    time1=par("time1");
    time2=par("time2");
    time3=par("time3");
    percHigh=par("percHigh");
    percMedium=par("percMedium");
    setLevel=par("setLevel");
    //enable_arb_pen=par("enable_arb_pen");
    scenario=par("scenario");
    prob=par("probability");
    typeOfDeadline=par("typeOfDeadline");
    swapper=par("swapper");
    nContestWin=0;
    globalPacket=0;
    underControl=false;
    indice=par("indice");

    //priority del nodo
    for(int i=0;i<8;i++){
        int random=uniform (0,100);
        if(random>50)
            priority[i]='1';
        else
            priority[i]='0';
    }
    if(!setLevel){
        int q1=uniform (0,100);
        int q2=uniform (0,100);
        int q3=uniform (0,100);

        if(q1>50)
            level1=true;
        if(q2>50)
            level2=true;
        if(q3>50)
            level3=true;
    }
    else{
        level1=par("level1");
        level2=par("level2");
        level3=par("level3");
    }

}

void EtherCatMACSlave::handleMessage(cMessage *msg)
{
    //EV << "I'm EtherCatMACSlave and handleMessage to gate "<< msg->getArrivalGate()->getFullName() << endl;

    if(msg->isSelfMessage()){
            EV << "I'm EtherCatMACSlave HM SELF node:"<< indice << "  and receive payload "<< msg <<" and resend at time "<< simTime().dbl()<< "\n";
            if(gate("phys2$o")->getNextGate()->isConnected()){
                send(msg->dup(),"phys2$o");
                EV << "I'm EtherCatMACSlave and send "<< msg << "to other Slave\n";
            }else{

                EV << "I'm EtherCatMACSlave and finish chain"<<endl;
                if(strcmp(msg->getName(),"END_PDU")==0){
                    //if(!enable_arb_pen)
                        if(controlIfIwon(msg))
                            EV << "Ho vinto \n";
                }
                send(msg->dup(),"phys1$o");
        }
    }

    else if(msg->getArrivalGate()==gate("phys1$i")){
       // EV << "I'm EtherCatMACSlave and receive single 1byte cPacket  "<< msg << ", is Global Value?:"<<  \n";
        EV << "I'm EtherCatMACSlave HM psy1 node:"<< indice <<" and receive single 1byte cPacket:" << msg->getName() << " at time "<< simTime().dbl()<<"\n";
        queueGenerator();
        if(strcmp(msg->getName(),"END_PDU")==0){


            //operazioni sull'indirizzo sequenziale
            cMsgPar *adp=&msg->par("ADP");
            long address=adp->longValue();
            address++;
            adp->setLongValue(address);

            //operazioni per recuperare il parametro di flag global
            //ovvero se il pacchetto vale per tutti i messaggi
            cMsgPar *global=&msg->par("global");
            bool globalValue=global->boolValue();
            if(globalValue){
                globalPacket++;
                ev << "Global Message arrived:" << absoluteDeadline << "\n";
                //if(underControl && enable_arb_pen)
                //    checkIfIWinOnArbPend(msg);
                if(!underControl)
                    setDeadlineOnFrame(msg);
                cPacket *byte = (cPacket*)msg;
                send(byte,"upperLayerOut");
            }
            else if(address==0){
                    cPacket *byte = (cPacket*)msg;
                    ev << "I'm EtherCatMACSlave and decapsulate payload lenght: "<<byte->getByteLength() << endl;
                    send(byte,"upperLayerOut");
                    EV << "I'm EtherCatMACSlave and send "<< byte << "to my upperLayerOut\n";
                }
            else{
                    scheduleAt(simTime()+delay, msg->dup());//era event
                    //scheduleAt(simTime()+uniform(0,1), event->dup());
                }
        }

        else{
            scheduleAt(simTime()+delay, msg->dup());//era event
            //scheduleAt(simTime()+uniform(0,1), event->dup());
        }
    }else if(msg->getArrivalGate()==gate("upperLayerIn")){
        scheduleAt(simTime()+delay, msg->dup());//era event
        //scheduleAt(simTime()+uniform(0,1), event->dup());


    }else if(msg->getArrivalGate()==gate("phys2$i")){
            if(strcmp(msg->getName(),"END_PDU")==0){
                //cMsgPar *deadl=&msg->par("deadl");
                //long deadlineValue=deadl->longValue();
                //EV << "Verifico se vinco relDed: "<< relativeDeadline << " dedVal: "<< deadlineValue << "\n";
                //if(!enable_arb_pen)
                    if(controlIfIwon(msg))
                        EV << "Ho vinto \n";

            }
            send(msg,"phys1$o");
                EV << "I'm EtherCatMACSlave and send "<< msg << "to prev slave\n";
        }
    else
        EV << "Error"<<endl;
}


bool EtherCatMACSlave::controlIfIwon(cMessage *msg){
    // verifico che ho scritto in qualche frame il mio valore, ovvero
    // che sto partecipando ad una contesa
    if(!underControl){
        return false;
    }
    if(swapper)
        return false;

    cMsgPar *timeStamp=&msg->par("timeStamp");
    double timeStampValue=timeStamp->doubleValue();
    if(timeStampValue==timeStart){
            return false;
    }

    // verifico che sto confrontando lo stesso frame che ho settato in partenza

    /*
    cTimestampedValue *timeStamp=(cTimestampedValue*)msg->getObject("END_PDU");
    if(timeStamp->timestamp!=timeStart){
        return false;
    }
    */
    // se siamo arrivati qui significa che stiamo controllando la frame giusta
    // sia che vinco sia che perdo, posso ripartecipare alle contese
    underControl=false;

    if(scenario==1){
        if(queue.length()>0){
            cMsgPar *queueHead= check_and_cast<cMsgPar*>(queue.get(0));
            double queueHeadValue=queueHead->doubleValue();
            cMsgPar *deadl=&msg->par("deadl");
            double deadlineValue=deadl->doubleValue();
            EV << "confronto Se ho vinto, la testa vale:"<< queueHead->doubleValue() << "\n";
            if(queueHeadValue==deadlineValue){
                nContestWin++;
                queueSched.insert(queue.pop());
                return true;
            }
        }
    }
    if(scenario==2){
        if(queue.length()>0){
            cMsgPar *queueHead= check_and_cast<cMsgPar*>(queue.get(0));
            std::string stringValue= queueHead->stringValue();
            cMsgPar *bitWise=&msg->par("cmd");
            cMsgPar *queueTimeStampHead= check_and_cast<cMsgPar*>(timeStampQueue.get(0));
            cMsgPar *deadl=&msg->par("deadl");
            std::string bitWiseValue=bitWise->stringValue();
            EV << "confronto Se ho vinto, la testa vale:"<< stringValue << " e il valore "<<bitWiseValue  <<"\n";
            if(stringValue==bitWiseValue && (queueTimeStampHead->doubleValue()==deadl->doubleValue())){
                nContestWin++;
                queueSched.insert(queue.pop());
                timeStampQueue.pop();
                return true;
            }
        }
    }
    return false;
}

void EtherCatMACSlave::setDeadlineOnFrame(cMessage *msg){


    if(scenario==1){
        cMsgPar *deadl=&msg->par("deadl");
        double deadlValue=deadl->doubleValue();
        EV << "I'm EtherCatMACSlave "<< indice<<" And I verified the Frame, deadValue: "<< deadlValue<< " \n";
        if(queue.length()>0){
            cMsgPar *queueHead= check_and_cast<cMsgPar*>(queue.get(0));
            EV << "confronto il frame passante:"<<deadlValue<<" , la testa vale:"<< queueHead->doubleValue() << "\n";
            if(deadlValue==0.0 || deadlValue>queueHead->doubleValue() ){
                // mi conservo il valore del timeStamp della frame
                // in maniera tale da poterla riconoscere al suo ritorno.
                cMsgPar *timeStamp=&msg->par("timeStamp");
                double timeStampValue=timeStamp->doubleValue();
                timeStart=timeStampValue;

                /*
                cTimestampedValue *timeStamp=(cTimestampedValue*)msg->getObject("END_PDU");
                timeStart= timeStamp->timestamp;//timeStampValue;
                */
                // setto una flag in maniera tale da non scrivere niente nelle frame successive

                if(swapper){
                    queueSched.insert(queue.pop());
                    if(deadlValue!=0.0){
                        sortQueue(deadl);
                    }// inserisco questo valore
                }
                else{
                    underControl=true;
                }
                EV << "I'm EtherCatMACSlave "<< indice<<" And I win the context and I write:" << queueHead->doubleValue() << "\n";
                deadl->setDoubleValue(queueHead->doubleValue());
            }
        }
    }
    if(scenario==2){
            cMsgPar *bitWise=&msg->par("cmd");
            std::string bitWiseValue=bitWise->stringValue();
            //EV << "I'm EtherCatMACSlave "<< node<<" And I verified the Frame, deadValue: "<< deadlValue<< " and relativeDeadLine: "<< relativeDeadline << "\n";
            if(queue.length()>0){
                /* con priorit� costante questa parte non c'�
                cMsgPar *queueHead= check_and_cast<cMsgPar*>(queue.get(0));
                std::string stringValue= queueHead->stringValue();
                if( bitWiseValue >stringValue){
                */
                cMsgPar *par= check_and_cast<cMsgPar*>(queue.get(0));

                if( bitWiseValue > par->stringValue()){
                    // mi conservo il valore del timeStamp della frame
                    // in maniera tale da poterla riconoscere al suo ritorno.

                    //cMsgPar *timeStamp=&msg->par("timeStamp");now
                    //double timeStampValue=timeStamp->doubleValue();
                    /*
                    cTimestampedValue *timeStamp=(cTimestampedValue*)msg->getObject("END_PDU");
                    EV << "PRIMA DI CRASHARE SCRIVI QUESTO2" << timeStamp->timestamp;
                    timeStart=timeStamp->timestamp;//timeStampValue;
                    */
                    //contestTimeStamp.insert(timeStamp->dup());
                    // setto una flag in maniera tale da non scrivere niente nelle frame successive
                    if(swapper){
                       queueSched.insert(queue.pop());
                       sortQueue(bitWise);
                       // inserisco questo valore
                       // a meno che non sia 11111111? posso usarlo per verificare se ho finito le
                       // richieste aperiodiche.
                   }
                   else{
                       underControl=true;
                   }

                    /*
                    EV << "I'm EtherCatMACSlave "<< node<<" And I win the context and I write:" << queueHead << "\n";
                    bitWise->setStringValue(queueHead->stringValue());
                    */
                    EV << "I'm EtherCatMACSlave "<< indice<<" And I win the context and I write:" << par->dup()->stringValue() << "\n";
                    bitWise->setStringValue(par->dup()->stringValue());
                    cMsgPar *deadl=&msg->par("deadl");
                    cMsgPar *timeStamp= check_and_cast<cMsgPar*>(timeStampQueue.get(0));
                    deadl->setDoubleValue(timeStamp->dup()->doubleValue());

                }
            }
        }
}

void EtherCatMACSlave::queueGenerator(){
    double random=uniform (0.0, 100.0,0.1);
    if(random>=prob)
        return;

    if(scenario==1){
        int random=uniform (0,100);

        //absoluteDeadline=uniform (simTime().dbl()+simTime().dbl()*0.01, simTime().dbl()+simTime().dbl()*0.1);
        if(random<percHigh && level1)
            absoluteDeadline=simTime().dbl()+time1;
        else if(random<(percMedium+percHigh) && level2)
            absoluteDeadline=simTime().dbl()+time2;
        else if(level3)
            absoluteDeadline=simTime().dbl()+time3;
        else
            return;

        cMsgPar *deadl=new cMsgPar("deadline");
        //EV <<"Simtime(): "<< simTime().raw()<<" relativeDeadline:"<<relativeDeadline<< " diff: "<<simTime().raw()-relativeDeadline;
        if(typeOfDeadline==2){
            deadl->setDoubleValue(absoluteDeadline-simTime().dbl());
        }
        if(typeOfDeadline==1){
            deadl->setDoubleValue(absoluteDeadline);
        }
        // SORTED
        sortQueue(deadl);
        /*FIFO
        queue.insert(relDeadl->dup());
        */
        queueTemp.insert(deadl->dup());
    }
    if(scenario==2){

        /* con uno scenario di valore variabile */
        int random=uniform (0,100);
        //char result[8]={'0','0','0','0','0','0','0','0'};
        //char temp[8];
        //itoa (relativeDeadline,temp,2);
        cMsgPar *bitWise=new cMsgPar("cmd");

        char h1[16]={'0','0','0','0','0','0','0','1'};
        for(int i=0;i<8;i++){
            h1[8+i]=priority[i];
        }
        char h2[16]={'0','0','0','0','1','1','1','1'};
        for(int i=0;i<8;i++){
            h2[8+i]=priority[i];
        }
        char h3[16]={'0','0','1','1','1','1','1','1'};
        for(int i=0;i<8;i++){
            h3[8+i]=priority[i];
        }

        if(random<percHigh && level1){
            bitWise->setStringValue(h1);
            absoluteDeadline=simTime().dbl()+time1;
        }
        else if(random<(percHigh+percMedium) && level2){
            bitWise->setStringValue(h2);
            absoluteDeadline=simTime().dbl()+time2;
        }
        else if(level3){
            bitWise->setStringValue(h3);
            absoluteDeadline=simTime().dbl()+time3;
        }
        else
            return;
        cMsgPar *timeStamp=new cMsgPar("timestamp");
        timeStamp->setDoubleValue(absoluteDeadline);
        //SORTED
        queueTemp.insert(bitWise);
        bornTimeStamp.insert(timeStamp);
        int i=sortQueue(bitWise);
        // FIFO
        //queue.insert(bitWise->dup());
        //queueTemp.insert(bitWise->dup());
        ev <<"\n\nI="<<i<<"\n";
        if(i>=0)
            timeStampQueue.insertBefore(timeStampQueue.get(i),timeStamp->dup());
        else
            timeStampQueue.insert(timeStamp->dup());
    }
    EV << "END QUEUE GENERATOR";
}

int EtherCatMACSlave::sortQueue(cMsgPar *msgPar){
    int i=0;
    if(scenario==1){
        double result=msgPar->doubleValue();
                bool finded=false;
                for(i=0;i<queue.length();i++){
                    cMsgPar *par= check_and_cast<cMsgPar*>(queue.get(i));
                    double value= par->doubleValue();
                    bool tester=value>result; // value maggiore di result
                    if(value>result){
                       ev <<" confronto result: "<<result<< " and value: "<< value << "result<value?--> "<<tester;
                       queue.insertBefore(queue.get(i),msgPar->dup());
                       finded=true;
                       break;
                    }

                }
               if(!finded){
                  queue.insert(msgPar->dup());
                  i=-1;
               }
               /*
                finded=false;
                for(int i=0;i<queueTemp.length();i++){
                   cMsgPar *par= check_and_cast<cMsgPar*>(queueTemp.get(i));
                   long value=  par->longValue(); // value � il valore del vettore
                   bool tester=value>result; // value maggiore di result
                   if(value>result){
                      //*temp=*par;
                       ev <<" confronto result: "<<result<< " and value: "<< value << "result<value?--> "<<tester;
                      queueTemp.insertBefore(queueTemp.get(i),msgPar->dup());
                      finded=true;
                      break;
                   }

                }
                if(!finded){
                   queueTemp.insert(msgPar->dup());
                }*/
        }
    // se costante non sar� mai chiamata
    if(scenario==2){
    const char* result=msgPar->stringValue();
            bool finded=false;
            for(i=0;i<queue.length();i++){
                cMsgPar *par= check_and_cast<cMsgPar*>(queue.get(i));
                const char* value= par->stringValue();
                bool tester=test(value,result); // value maggiore di result
                if(tester){
                   ev <<" confronto result: "<<result<< " and value: "<< value << "result<value?--> "<<tester;
                   queue.insertBefore(queue.get(i),msgPar->dup());
                   finded=true;
                   break;
                }

            }
           if(!finded){
              queue.insert(msgPar->dup());
              i=-1;
           }
           /*
            finded=false;
            for(int i=0;i<queueTemp.length();i++){
               cMsgPar *par= check_and_cast<cMsgPar*>(queueTemp.get(i));
               const char* value=  par->stringValue(); // value � il valore del vettore
               bool tester=test(value,result); // value maggiore di result
               if(tester){
                  //*temp=*par;
                   ev <<" confronto result: "<<result<< " and value: "<< value << "result<value?--> "<<tester;
                  queueTemp.insertBefore(queueTemp.get(i),msgPar->dup());
                  finded=true;
                  break;
               }

            }
            if(!finded){
               queueTemp.insert(msgPar->dup());
            }*/
    }
    return i;
}

/*
void EtherCatMACSlave::checkIfIWinOnArbPend(cMessage *msg){
        // controllo se ho vinto al token successivo con CAN interfac
            cMsgPar *timeStamp=&msg->par("timeStampPrevius");
            long timeStampValue=timeStamp->longValue();
            timeStampQueue.insert(timeStamp->dup());
            EV <<"--->checkIfIWinOnArbPend\n";
            EV << "Confronto time1:"<<timeStampValue<<" e time2:"<<timeStart;

            // verifico che sto confrontando lo stesso frame che ho settato in partenza
            if(timeStampValue!=timeStart){
                return;
            }
            cMsgPar *queueHead= check_and_cast<cMsgPar*>(queue.get(0));
            std::string stringValue= queueHead->stringValue();
            cMsgPar *bitWise=&msg->par("cmdPrevius");
            std::string bitWiseValue=bitWise->stringValue();
            EV << "ARB-PEND->confronto Se ho vinto, la testa vale:"<< stringValue << " e il valore "<<bitWiseValue  <<"\n";
            if(stringValue==bitWiseValue){
                nContestWin++;
                queueSched.insert(queue.pop());
            }

            EV << "ARB-PEND->Ho Vinto\n";
            underControl=false;
            return;
}
*/
bool EtherCatMACSlave::test(const char* a,const char* b){
    // metodo che serve a verificare che un numero binario sia maggiore di un altro
    for(int i=0;i<8;i++){
        if(a[i]>b[i]){
            //ev << "a["<<i<<"]=" <<a[i] << ">b["<<i<<"]"<<b[i];
            return true;
        }
        if(a[i]<b[i]){
            //ev << "a["<<i<<"]=" <<a[i] << "<b["<<i<<"]"<<b[i];
            return false;
        }
    }
    return false;
}


void EtherCatMACSlave::finish(){

        EV << "### NODE: "<< indice<<"\n";
        EV << "level1 active: "<< level1<<"\n";
        EV << "level2 active: "<< level2<<"\n";
        EV << "level3 active: "<< level3<<"\n";
        EV << "I received " << globalPacket << " globalPacket"<< "\n";
        EV << "I win "<< nContestWin << " time/s a contest \n";
        EV <<"FRAME GENERATE: "<<queueTemp.length()<< "\n";
        EV <<"FRAME SCHEDULATE: "<<queueSched.length()<< "\n";
        EV <<"FRAME RIMASTE IN CODA: "<<queue.length()<< "\n";

        if(scenario==1){
            EV <<"LISTA FRAME GENERATE:[";
            for(int i=0;i<queueTemp.length();i++){
                cMsgPar *par= check_and_cast<cMsgPar*>(queueTemp.get(i));
                EV <<  par->doubleValue();
                if(i+1<queueTemp.length()){
                    EV << ",";
                }
            }
            EV <<"]\n";
            EV <<"LISTA FRAME LISTA FRAME SCHEDULATE:[";
                for(int i=0;i<queueSched.length();i++){
                   cMsgPar *par= check_and_cast<cMsgPar*>(queueSched.get(i));
                   EV <<  par->doubleValue();
                   if(i+1<queueSched.length()){
                       EV << ",";
                   }
             }
             EV <<"]\n";
            EV <<"LISTA FRAME RIMASTE IN CODA:[";
            for(int i=0;i<queue.length();i++){
               cMsgPar *par= check_and_cast<cMsgPar*>(queue.get(i));
               EV <<  par->doubleValue();
               if(i+1<queue.length()){
                   EV << ",";
               }
           }
           EV <<"]\n\n";

        }
        if(scenario==2){
            EV <<"LISTA FRAME GENERATE:[";
            for(int i=0;i<queueTemp.length();i++){
                cMsgPar *par= check_and_cast<cMsgPar*>(queueTemp.get(i));
                for(int k=0;k<16;k++){
                    EV <<  par->stringValue()[k];
                }
                if(i+1<queueTemp.length()){
                    EV << ",";
                }
            }
            EV <<"]\n";
            EV <<"LISTA FRAME SCHEDULATE:[";
            for(int i=0;i<queueSched.length();i++){
                cMsgPar *par= check_and_cast<cMsgPar*>(queueSched.get(i));
                for(int k=0;k<16;k++){
                    EV <<  par->stringValue()[k];
                }
                if(i+1<queueSched.length()){
                    EV << ",";
                }
            }
            EV <<"]\n";
            EV <<"LISTA FRAME RIMASTE IN CODA:[";
            for(int i=0;i<queue.length();i++){
                cMsgPar *par= check_and_cast<cMsgPar*>(queue.get(i));
                for(int k=0;k<16;k++){
                    EV <<  par->stringValue()[k];
                }
                if(i+1<queue.length()){
                    EV << ",";
                }
            }
            EV <<"]\n";
               EV <<"LISTA BORN TIMESTAMP CONFRONTO:[";
               for(int i=0;i<bornTimeStamp.length();i++){
                  cMsgPar *par= check_and_cast<cMsgPar*>(bornTimeStamp.get(i));
                  EV <<  par->doubleValue();
                  if(i+1<bornTimeStamp.length()){
                      EV << ",";
                  }
              }
            EV <<"]\n";
                EV <<"LISTA TIMESTAMP CONFRONTO:[";
                for(int i=0;i<timeStampQueue.length();i++){
                   cMsgPar *par= check_and_cast<cMsgPar*>(timeStampQueue.get(i));
                   EV <<  par->doubleValue();
                   if(i+1<timeStampQueue.length()){
                       EV << ",";
                   }
               }

             EV <<"]\n\n";
        }

}
