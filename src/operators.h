
#pragma once

#include "ofMain.h"
//addons
#include "json.h"
#include "ofxMSAInteractiveObjectDelegate.h"
//using this branch of ofxMSAInteractiveObject: https://github.com/Flightphase/ofxMSAInteractiveObject.git
#include "ofxTimeline.h" 
//from https://github.com/YCAMInterlab/ofxTimeline. this one has additional dependencies of its own...

//forward declarations 
class testApp;
class ofxOperator; 
class ofxOperatorUI;
class ofxDuctBase;
class ofxDuctUI;


class ofxOperator{
 public:
  int uid;  //unique within graph.
  string operatorType;
  string helpInfo; // tooltip for your op. 
     
  ofxOperatorUI * uiElement;
     
  //used for type checking ducts while connecting, and for display
  map<string, string>  inputTypes; //name, type
  map<string, string>  outputTypes; //name, type
  //should be populated at instantiation... 
    
  //for managing ducts ... 
  virtual void bindInput(string inputName, ofxDuctBase* duct); //up to you to deal with this.  
  virtual ofxDuctBase* getOutput(string outputName); //give us a connection.
  //
  virtual void clearInput(string inputName);
  list<pair<string,string> > revealFreeInputs(); //for generating UI widgets for unbound paramaters. 
    
  virtual void run(); //do your thing here.
  virtual ofxOperator * clone(){ return new ofxOperator();}; //make this pure virtual at some point.
  virtual void loadJSON(Json::Value initValues){}; //for loading object state.
  virtual Json::Value saveJSON(){return new Json::Value();}; //for saving state. 
  virtual void uiAvailable(){}; //called when ui is populated so you can add op-specific extras
  virtual ~ofxOperator(){}; //lets avoid memory leaks. 

    //DAG stuff handled in the controller. 
    

};
class ofxDuctBase{
 public:
  ofxDuctBase(){
        
  }
  virtual string getTypeName(){ return "baseDuct";}; //maybe make this pure virtual later.. 
};
class ductb : public ofxDuctBase{
 public:
  virtual string getTypeName(){ return typeid(*this).name();};
};

template<class T> 
class ofxDuct : public ofxDuctBase{
 public: 
  T value;
  string name;
  virtual string getTypeName() {return name;};
};

class textFieldWithDelegate : public ofxMSAInteractiveObjectWithDelegate{
public:
  
  ofxTextInputField textfield;
  textFieldWithDelegate(){
    textfield.setup();
  }  
  void draw(){
    //ofxMSAInteractiveObjectWithDelegate::draw();
    textfield.bounds.x = x;
    textfield.bounds.y = y;
    textfield.draw();
  }
};

class ofxDuctUI : public ofxMSAInteractiveObjectWithDelegate{
 public:
  ofxOperator * op;
  ofxDuctUI * inputLink; 
  bool isInput;
  void draw();
};
class ofxOperatorUI : public ofxMSAInteractiveObjectWithDelegate{
 public:
  ofxOperator * op;

  list<ofxMSAInteractiveObject*> children; //to cascade movement.

  virtual ~ofxOperatorUI() {
    for(map<string, ofxDuctUI*>::iterator it = inputs.begin(); it!=inputs.end(); it++){
      delete (it->second);
    }
    for(map<string, ofxDuctUI*>::iterator it = outputs.begin(); it!=outputs.end(); it++){
      delete (it->second);
    }

    cout << "base destructor called" <<endl;
  }; 
  map<string, ofxDuctUI*> inputs; //ez access to children. 
  map<string, ofxDuctUI*> outputs;

  virtual void draw();
  virtual void setPos(float _x, float _y){        
    float dx = _x - x;
    float dy = _y - y;
    x += dx;
    y += dy;
    for (list<ofxMSAInteractiveObject*>::iterator it = children.begin(); it != children.end(); it++){
      (**it).x += dx;
      (**it).y += dy;
      cout << "has children" << endl;
    }
  }
  void populate(ofxMSAInteractiveObjectDelegate* delegate);
};


template <class T>
  class producerOp : public ofxOperator{
  ofxDuct<T> out;
  T (*fun)();
 public: 
  producerOp(){};
  producerOp(string opName, string outName, string outType, T (*functionRef)()){
    operatorType= opName;
    outputTypes.insert(make_pair(outName,outType));
    fun = functionRef;
  }     
  virtual ofxDuctBase* getOutput(string outputName){
    return &out;
  }
  //no bindinput;
  void run(){
    out.value=fun();
  }
  producerOp* clone(){
    producerOp* temp = new producerOp();
    (*temp)=(*this); //copy. 
    return temp;
  }
 };

class floatTL : public ofxOperator{
  ofxDuct<float> out;
  ofxTimeline* maintimeline;
  ofxTLCurves* curveref;
  textFieldWithDelegate tlname;
  string oldname;
 public:
  floatTL(ofxTimeline* mainTimeline, string opName){
        maintimeline = mainTimeline;
        operatorType = opName;
	outputTypes.insert(make_pair("float","float"));
	curveref= NULL;
	oldname= "";
  }
   ~floatTL(){
     maintimeline->removeTrack(oldname);
  }

  floatTL* clone(){
    cout << operatorType << endl;
    floatTL* temp = new floatTL(maintimeline,operatorType);    
    temp->out.value = 0;       
    return temp;
  }
  void uiAvailable(){
    tlname.textfield.autoClear = TRUE;
    if(tlname.textfield.text.length()==0){ 
      tlname.textfield.text="name me!";
    }else{
        createTLElement(tlname.textfield.text);
    }
    tlname.setPosAndSize(uiElement->x, uiElement->y-30, 20, 20);
    uiElement->children.push_back(&tlname);
    ofAddListener(tlname.textfield.textChanged, this, &floatTL::createTLElement);
  }
  void loadJSON(Json::Value initValues){
    tlname.textfield.text = initValues["floatTLname"].asString();
    oldname = tlname.textfield.text;
  }
  Json::Value saveJSON(){
    Json::Value temp;
    temp["floatTLname"] = tlname.textfield.text;
    return temp;
  }
  void createTLElement(string & name){
     maintimeline->removeTrack(oldname);
    curveref = maintimeline->addCurves(name);
    oldname = name;
  }
  ofxDuctBase* getOutput(string outputname){
    return &out;
  }
  //no bind input
  void run(){
    tlname.draw();
    if (curveref != NULL){
      out.value = curveref->getValue();
    }
  } 
};

class colorTL : public ofxOperator{
  ofxDuct<ofColor> out;
  ofxTimeline* maintimeline;
  ofxTLColorTrack* colorref;
 public:
  colorTL(ofxTimeline* mainTimeline, string opName){
        maintimeline = mainTimeline;
        operatorType = opName;
	outputTypes.insert(make_pair("ofColor","ofColor"));
	colorref= NULL;
  }
   ~colorTL(){
     maintimeline->removeTrack("colorTL");
  }

  colorTL* clone(){
    cout << operatorType << endl;
    colorTL* temp = new colorTL(maintimeline,operatorType);

    temp->colorref = maintimeline->addColors("colorTL");
    temp->out.value = ofColor(0,0,0);
    return temp;
  }
  ofxDuctBase* getOutput(string outputname){
    return &out;
  }
  //no bind input
  void run(){
    if (colorref != NULL){
      out.value = colorref->getColor();
    }
  } 
};

template <class T, class U>
  class unaryOp : public ofxOperator{
  ofxDuct<T>* in;
  ofxDuct<U> out;
  U (*fun)(T);
 public: 
  unaryOp(){};
  unaryOp(string opName, string inName, string inType, string outName, string outType, U (functionRef)(T)){
    operatorType= opName;
    in = NULL;
    inputTypes.insert(make_pair(inName,inType));
    outputTypes.insert(make_pair(outName,outType));
    fun = functionRef;
  }
  virtual ofxDuctBase* getOutput(string outputname){
    return &out;
  }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if (duct!=NULL){in = dynamic_cast<ofxDuct<T> *>(duct);}else{in=NULL;};
  }
  void run(){
    if(in!=NULL){
      out.value=fun(in->value);
    }
  }
  unaryOp* clone(){
    unaryOp* temp = new unaryOp();
    (*temp)=(*this); //copy. 
    return temp;
  }
};
	  
class ofPointCon : public ofxOperator{
  ofxDuct<int>* x;
  ofxDuct<int>* y;
  ofxDuct<ofPoint> point;
	  
 public:
  ofPointCon(){
    x=NULL;
    y=NULL;
    point.value = ofPoint(0,0);

    operatorType = "ofPoint";

    inputTypes.insert(make_pair("x", "int"));
    inputTypes.insert(make_pair("y", "int"));
    outputTypes.insert(make_pair("ofPoint", "ofPoint"));
  }
  virtual ofxDuctBase* getOutput(string outputName){
    return &point;
      }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if (inputName.compare("x")==0){
      if (duct != NULL){x= dynamic_cast<ofxDuct<int> *>(duct); cout << "woot" << endl;}else{x=NULL;};
    }
    
    if (inputName.compare("y")==0){
      if (duct!=NULL){y= dynamic_cast<ofxDuct<int> *>(duct);}else{y=NULL;};
    }
  }


  void run(){
    if((x!=NULL)&&(y!=NULL)){point.value.x = x->value; point.value.y = y->value;}else{point.value.x=50; point.value.y=80;}
  }
  virtual ofPointCon* clone(){
    return new ofPointCon();
  }
};

class intRGBAColor : public ofxOperator{
  ofxDuct<int>* r;
  ofxDuct<int>* g;
  ofxDuct<int>* b;
  ofxDuct<int>* a;
  ofxDuct<ofColor> color;
 public:

  intRGBAColor(){
  r= NULL;
  g= NULL;
  b= NULL;
  a= NULL;

  operatorType = "intRGBAColor";

  inputTypes.insert(make_pair("r", "int"));
  inputTypes.insert(make_pair("g", "int"));
  inputTypes.insert(make_pair("b", "int"));
  inputTypes.insert(make_pair("a", "int"));
  outputTypes.insert(make_pair("ofColor", "ofColor"));
  }
  ofxDuctBase * getOutput(string outputName){
    return &color;
  }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if (inputName.compare("r")==0){
      if (duct!=NULL){r = dynamic_cast<ofxDuct<int> *>(duct);}else{r=NULL;};
    }
    if (inputName.compare("g")==0){
      if (duct!=NULL){g = dynamic_cast<ofxDuct<int> *>(duct);}else{g=NULL;};
    }
    if (inputName.compare("b")==0){
      if (duct!=NULL){b = dynamic_cast<ofxDuct<int> *>(duct);}else{b=NULL;};
    }
    if (inputName.compare("a")==0){
      if (duct!=NULL){a = dynamic_cast<ofxDuct<int> *>(duct);}else{a=NULL;};
    }
  }
  void run(){
     color.value = ofColor(0,0,0,255); //defaults to black full alpha
    if (r!=NULL) color.value.r = r->value;
    if (g!=NULL) color.value.g = g->value;
    if (b!=NULL) color.value.b = b->value;
    if (a!=NULL) color.value.a = a->value;
  }
  virtual intRGBAColor* clone(){
    return new intRGBAColor();
  }
};


class drawCircle : public ofxOperator{
  ofxDuct<int>* radius;
  ofxDuct<ofColor>* color;
  ofxDuct<ofPoint>* location;

 public: 
  drawCircle(){
    radius= NULL;
    color= NULL;
    location = NULL;

    
    operatorType= "drawCircle";
    inputTypes.insert(make_pair("radius", "int"));
    inputTypes.insert(make_pair("ofColor", "ofColor"));
    inputTypes.insert(make_pair("location", "ofPoint"));
  }

  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if (inputName.compare("radius")==0){
      if(duct != NULL){radius = dynamic_cast<ofxDuct<int> *>(duct);}else{radius=NULL;};
  }
    if (inputName.compare("ofColor")==0){
      if(duct != NULL){color = dynamic_cast<ofxDuct<ofColor> *>(duct);}else{color=NULL;};
  }
    if (inputName.compare("location")==0){
      if(duct != NULL){location = dynamic_cast<ofxDuct<ofPoint> *>(duct);}else{location=NULL;};
  }
  }
  
  void run(){
    if(color!=NULL){
      ofSetColor(color->value);
    }
    if((radius!=NULL)&&(location!=NULL)){
      ofCircle(location->value.x,location->value.y,radius->value);
    }
  }
  drawCircle* clone(){
    return new drawCircle();
  }
};

class addInts : public ofxOperator{
  //ref the inputs, own the outputs.
  ofxDuct<int>* a;
  ofxDuct<int>* b;
  ofxDuct<int> output;
 public:    

  addInts(){
    a = NULL;
    b = NULL;
    operatorType = "addInts";
    inputTypes.insert(make_pair("a", "int")); 
    inputTypes.insert(make_pair("b", "int")); 
    outputTypes.insert(make_pair("sum", "int"));
  }
  virtual ofxDuctBase* getOutput(string outputName){
    return &output;
  }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    cout << "bindInput name:" << inputName << endl;
    cout << "bindInput ductNull: "<< (duct==NULL) << endl;
    if (inputName == "a"){
      if (duct!=NULL){a = dynamic_cast<ofxDuct<int> *>(duct);}else{a=NULL;}; 
      cout << "a dynamic cast worked:" << (a!=NULL) << endl;
    }
    if (inputName == "b"){
      if (duct!=NULL){b = dynamic_cast<ofxDuct<int> *>(duct);}else{b=NULL;};
      cout << "b dynamic cast worked:" << (a!=NULL) << endl;
    }
  }
    
  void run(){
    if ((a!=NULL)&&(b!=NULL)){
      output.value = a->value + b->value;
    } else {
      output.value = 120;
    }
  }
  virtual addInts* clone(){
    return new addInts();
  }
};

class showInt : public ofxOperator{
  ofxDuct<int>* a;
  ofxMSAInteractiveObjectWithDelegate  visual;
 public:
  showInt(){
    a = NULL;
    operatorType = "showInt";
    inputTypes.insert(make_pair("a", "int"));
    // outputTypes.insert(make_pair("a", "int"));


  }
  virtual showInt* clone(){
    return new showInt();
  }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if ((inputName == "a")&&duct!=NULL){
      a = dynamic_cast<ofxDuct<int> *>(duct); 
      cout << "a dynamic cast worked:" << (a!=NULL) << endl;

 
     visual.setup();
      visual.setLabel("0");
      if (uiElement!=NULL) {
	visual.setPosAndSize(uiElement->x, uiElement->y-30, 40, 20);
	cout<< "uielement non null" <<endl;
	cout<< "uielement x" << uiElement->x <<endl;
	uiElement->children.push_back(&visual);
      }

    }
  }
  void run(){
    if (a!= NULL){
      visual.setLabel(ofToString(a->value));
    }
  }
  
};

class _cout : public ofxOperator{
  ofxDuct<string>* a;
  ofxMSAInteractiveObjectWithDelegate visual;
 public:
  _cout(){
    a = NULL;
    operatorType = "std::cout";
    inputTypes.insert(make_pair("a", "string"));
  }
  virtual _cout* clone(){
    return new _cout();
  }
  virtual void bindInput(string inputName, ofxDuctBase* duct){
    if ((inputName == "a")&&duct!=NULL){
      a = dynamic_cast<ofxDuct<string> *>(duct); 
      cout << "a dynamic cast worked:" << (a!=NULL) << endl;
            
      visual.setup();
      visual.setLabel("0");
      if (uiElement!=NULL) {
	visual.setPosAndSize(uiElement->x, uiElement->y-30, 40, 20);
	cout<< "uielement non null" <<endl;
	cout<< "uielement x" << uiElement->x <<endl;
	uiElement->children.push_back(&visual);
      }
            
    }
  }
  void run(){
    if (a!= NULL){
      visual.setLabel(a->value);
      cout << a-> value << endl;
    }
  }
};

//just a testing class for input.
class constInt : public ofxOperator{
  ofxMSAInteractiveObjectWithDelegate visual;
  ofxDuct<int> output;
 public:
  int value;
  constInt(){

    operatorType = "constInt";

    outputTypes.insert(make_pair("a", "int"));
    value =0;
        
  }
  virtual constInt* clone(){
    return new constInt();
  }
    
  virtual ofxDuctBase* getOutput(string outputName){
    return &output;
  }
  //    virtual void bindInput(string inputName, ofxDuctBase* duct){    }
  void run(){
    output.value = value;
    visual.setLabel(ofToString(value));
  }
};
