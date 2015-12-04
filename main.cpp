#include "tbapi.h"    
#include <iostream>
#include <cstdlib>        
#include <string> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>                                                            // main API header file

#include "TUIO/TuioServer.h"
#include "TUIO/TuioCursor.h"
// The callback function
using namespace TUIO;

#define THRESH_TOUCH .05
#define THRESHTIME .2

#define TOUCHSIZE 12

#define DBG(x) std::cout << x << std::endl;
#define DBGN(x) std::cout << x ;

#include <ctime>


double getTime() {
  using namespace std;
  clock_t begin = clock();
  return  double( begin)*100.0 / CLOCKS_PER_SEC;

}


void myExit(int i);

typedef struct point{
  float x;
  float y;
  double pressTime ; 
  bool active;
  bool newOne;

  TuioCursor * cursor;

  point(){
    active = false;
    newOne = false;
    pressTime = 0;

  };
 
}point;

HTBDEVICE device;
static point points[TOUCHSIZE],myPoints[TOUCHSIZE];

static int numTouch = 0;

TuioServer tuioServer = TuioServer();

float sqdistance(point & a,point & b){
  return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

void displayTouch(int idx,point p){
  DBG(idx << " : " << p.x << "," << p.y);

}


void updateTouches(float x, float y){
  bool found = false;
  point cP ;
  cP.x=x;
  cP.y=y;
  cP.active = true;
  cP.newOne = false;
  cP.cursor = NULL;
  double curTime = getTime();
  cP.pressTime = curTime;
  // DBG(curTime);
  for(int i = 0 ; i < numTouch ; i++){
    float dist = sqdistance(cP,points[i]);
    //DBG(dist <<","<< THRESH_TOUCH)
    if(points[i].active && dist < THRESH_TOUCH){
      points[i].x = x;
      points[i].y = y;
      points[i].active = true;
      points[i].pressTime = curTime;
      points[i].newOne = false;
      DBG("found: " <<i << "," <<  curTime);
      displayTouch(i,points[i]);
      
      return;
      }
  }

  if(numTouch < TOUCHSIZE){
    cP.newOne = true;
    points[numTouch] = cP;

      for(int i = 0 ; i < numTouch ; i++){
        float dist = sqdistance(cP,points[i]);
        DBGN(points[i].x << ",");
      }
      DBG("adding : " << numTouch << ": at " << points[numTouch].pressTime);
    // displayTouch(numTouch,points[numTouch]);
    

  return;
}

DBG("dropped : " <<numTouch<< cP.x<<","<<cP.y);
}

void updateTuio(){

  double curTime = getTime();
  bool falt = false;;
  for(int i = 0 ; i < TOUCHSIZE ; i++){
    if(myPoints[i].newOne){
      DBG("newT : " <<i<<", "<<curTime <<","<< myPoints[i].pressTime );
      myPoints[i].cursor = tuioServer.addTuioCursor(myPoints[i].x,myPoints[i].y);
      myPoints[i].newOne = false;
      numTouch++;
    }
    else if( myPoints[i].active && myPoints[i].cursor!=NULL){
      if( falt || curTime - myPoints[i].pressTime>THRESHTIME){
        
        DBG("tmeOUtT : " <<i<<", "<<curTime <<","<< myPoints[i].pressTime );
        tuioServer.removeTuioCursor(myPoints[i].cursor);
        myPoints[i].cursor = NULL;
        myPoints[i].active = false;
        myPoints[i].newOne = false;

      }
      else {
        tuioServer.updateTuioCursor(myPoints[i].cursor,myPoints[i].x,myPoints[i].y);
      }
    }
  else{
  		if(!falt){
		numTouch =i;
	}
    falt = true;

    
    // DBGN(i << " : " <<myPoints[i].active <<","<< curTime <<","<<myPoints[i].pressTime << "/" );

  }
}
if(myPoints[0].active){
  // DBG(curTime <<","<< points[0].pressTime);
}
}


void TBAPI fnCallBack(unsigned long context, _PointerData* data)
{
  // std::cout << "recieved : " << data->type<< "      " << std::endl; ;
            if(data->type == _ReadDataTypeXYCal)
            {
              updateTouches((int)data->pd.xy.calx/65535.0,(int)data->pd.xy.caly/65535.0);
              memcpy(myPoints,points,sizeof(point)*TOUCHSIZE);  
            }

            else if(data->type == _ReadDataTypeZ){
                std::cout <<" z=" <<(int)data->pd.z.rawz << std::endl; 
            }
            else if(data->type == _ReadDataTypeEvent){
              DBG((int)data->pd.event.left <<"  / " <<(int)data->pd.event.right << "," << data->pd.event.timed);
            }
            else if(data->type == _ReadDataTypeRawEvent){

              if(data->pd.rawEvent.eventHandle == 0 ){

                // numTouch += data->pd.rawEvent.state?1 : -1;

              }
                DBG(data->pd.rawEvent.eventHandle << " / " << // handle to event object if -1 this callback indicates the end of 
              
           // event bit processing for the packet - and the state flag below is not  populated 
              (int)data->pd.rawEvent.state<< " / " <<   // state of event 1 = set
              data->pd.rawEvent.timed<< " / " <<
               data->pd.rawEvent.wasToolbarTouch<< " / " <<
               data->pd.rawEvent.gesture)
            }
            else if(data->type==_ReadDataTypeData){
              for(int i = 0 ; i < 64 ; i++){
                DBGN((int)data->pd.data.byte[i]);
              }
              DBG("");

            }
            // else if (data->type == _ReadDataTypePhysicalEvent ){
            //  DBG("physical : " << data->pd.physicalEvent.state); 
            // }
            else if(data->type == _ReadDataTypeUnload)                    // driver is unloading!
            { 
             // begin exit process…
              exit(0);
            } 

            
}
// At startup
int main(int a,char** args){
  

  signal(SIGINT, myExit);
  printf("start");
  std::string path = "/Library/Preferences/";
  
  char dpath[1024];
  TBApiDefaultSettingsPath(dpath,sizeof(dpath));
  std::cout << "path is : " <<dpath << std::endl;
  std::cout << "init : " << TBApiInitEx(dpath) << std::endl;
                                                                  // initialise the API 
  std::cout << "opening : " << TBApiOpen()<< std::endl;
  device = TBApiGetRelativeDevice(0);
                                                                        // establish driver connection
  if(device != TB_INVALID_HANDLE_VALUE){
   std::cout << "device found : " << (int)device<< std::endl;
   TBApiRegisterDataCallback(device,         // first device on system
   0,                                                                                      // no context data
   0xFFFFFFFFFFF,//_ReadDataTypeXY | _ReadDataTypeUnload | _ReadDataTypeZ,// |_ReadDataTypePhysicalEvent,                         // 'OR' type(s) of data to return
   fnCallBack); 
  }
  else{
   std::cout << "no device found : " << device<< std::endl;
   exit(0);
  }

  // TBApiRawDataMode(device, true);         
  //          TBApiRawDataModeBlockSize(device , 1);
  TBApiMousePortInterfaceEnableEx(device,0);
  
  int delta = 1000;//1000;

  int count = 0;
 
  numTouch = 0;
  TuioTime currentTime = 0;
  
  currentTime.initSession();
  tuioServer.setVerbose(true);
            while(1){
                          
              updateTuio();
              memcpy(points,myPoints,sizeof(point)*TOUCHSIZE);    
              //tuioServer.stopUntouchedMovingCursors();
              tuioServer.commitFrame();
              currentTime = TuioTime::getSessionTime();
              tuioServer.initFrame(currentTime);
              
              // currentTime = TuioTime::getSessionTime();
              // tuioServer.initFrame(currentTime);
              //   DBG("t" << currentTime.getMicroseconds())
                
              //   // tuioServer.stopUntouchedMovingCursors();
              //   tuioServer.commitFrame();
                

                usleep(delta);

            } 

            return (0);                                                                     // address of callback function above
}
 
// At exit…
 
void myExit(int t){ 
  DBG("finishing");
  TBApiRawDataMode(device, false);
  TBApiUnregisterDataCallback(fnCallBack);                           // unregister functions before exit
  TBApiClose();                                                                   // close the driver connection
  TBApiTerminate();
  TBApiMousePortInterfaceEnableEx(device,1);
  exit(0);
                                                             // and conclude use of API
}
