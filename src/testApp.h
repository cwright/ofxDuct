// (c) colin wright 2012, MIT licence. 

#pragma once

#include "ofMain.h"

//addons
//---------------
#include "json.h"
//from jsoncpp

#include "ofxMSAInteractiveObjectDelegate.h"
//using this branch of ofxMSAInteractiveObject: https://github.com/cwright/ofxMSAInteractiveObject.git

#include "ofxTimeline.h" 
//from https://github.com/YCAMInterlab/ofxTimeline. this one has additional dependencies of its own...

#include "operators.h"
//stl
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <set>

//notes for future....
//#include <typeinfo>
//#include "cxxabi.h"
//cxxabi is for run time type inference, could bypass requiring inputing typenames manually in addPrototype().

 class endPointEventHandler : public ofxMSAInteractiveObjectDelegate {
 public:
  list<ofxDuctUI*>connectionCandidates;
    
  testApp * controller;
  virtual void objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y);    
};

class newOperatorEventHandler : public ofxMSAInteractiveObjectDelegate {
 public:
  testApp * controller;
  virtual void objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y);
 private:
    
};

class testApp : public ofBaseApp, public ofxMSAInteractiveObjectDelegate{
 public:
  ofxTimeline timeline;
  //baseApp stuff
  void setup();
  void update();  
  void draw();
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseDragged(int x, int y, int button);
  void keyPressed(int key); 
  bool keyboardFocus;
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
  
  //MSA obj delegate stuff
  virtual void objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y);
  virtual void objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button);	
  virtual void objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y);
    
  //duct controller stuff
    
  //uid vars
  list<int> freeUIDs; 
  int uidCounter;

  map<int, ofxOperator*> opGraph; //uid, node-- main graph

  multimap<int, pair<int, string> > outputLinks; //unneeded for DAG solver, just simple lookup for deleting ops. 
  //  out-uid,    in-uid, inputName
  map<int, pair<int, map<string, int> > > dag; //working graph for DAG
  //uid,   num out,   inputName, uid of linked

  list<int> zeroedNodes; 
  list<ofxOperator*> runQueue; 
    
  map<string, ofxOperator*> opPrototypes; //typename, derivedtype. 
  //using a map ensures that every operator typename is unique on entry and cannot be overridden 
  list<ofxMSAInteractiveObjectWithDelegate*> protoButtons;

  void addPrototype(ofxOperator* newProto); //adds operator prototype to namespace of available operators 

  void addOperatorFromUI(string opType, ofPoint location); // instantiate and add to graph. 
  void addOperatorFromJson(string opType, ofPoint location, int uid, Json::Value opData);
    
  int generateUID(); 

  void newOpToGraph(ofxOperator* theOp, int uid); //used internally.
  void removeOp(ofxOperator* theOp); //cleans up its connections as well. 

  void addDuctFromUI(list<ofxDuctUI*>* candidates);
  void addDuctFromJson(int fromUid, string fromOutputName, int toUid, string toInputName);

  void openJSON(string path);
  void saveJSON(string path);
    
  bool tsortCheckDuct(int from, int to, string toInput);
    
  void removeDuctFromDAG(int from, int to, string toInput);
  void removeDuctFromOutputLinks(int from, int to, string toInput);
  void updateDAG();
    
  bool typeCheckDuct(ofxOperator* from, string outputName, ofxOperator* to, string inputName);   
    
  ofxMSAInteractiveObjectWithDelegate obj;
  ofxMSAInteractiveObjectWithDelegate objb;
  ofPoint cursor;
  list<ofxMSAInteractiveObject*> dragQueue;
  list<ofxMSAInteractiveObject*> endPointQueue;

  list<ofxMSAInteractiveObjectWithDelegate*> endPoints;
  //    list<ofxDuctUI*> dendPoints;    
  ofxDuctUI* selectedDuct;
  ofxOperatorUI* selectedOp;
  ofPoint dragFrom;
  bool dragging;
  float px,py;
  endPointEventHandler EPhandler;
  newOperatorEventHandler newOpHandler;
  void showProtoMenu();
  void hideProtoMenu();
  void drawCursor(ofPoint thePoint);    

};
