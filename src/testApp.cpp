// (c) colin wright 2012, MIT licence. 

#include "testApp.h"

string intToString(int theInt){
  return ofToString(theInt);
} 

int scaleInt(float theFloat){
  return (int)(255 * theFloat);
}

void testApp::setup() {
  ofSetVerticalSync(false);
  timeline.setup();
  timeline.setDurationInSeconds(60); //sets time
  timeline.setLoopType(OF_LOOP_NORMAL); //turns the timeline to loop

  cursor = ofPoint( 100,100);
  keyboardFocus = true;
  ofEnableSmoothing();
  ofEnableBlendMode(OF_BLENDMODE_ALPHA);
  ofSetFrameRate(30);
  dragging =false;

  EPhandler.controller = this;
  newOpHandler.controller = this;
  uidCounter = 0;    
    
  //  instantiate prototype to make your operatortype available at runtime. 
  addPrototype(new addInts); 
  addPrototype(new showInt);
  addPrototype(new constInt);
  addPrototype(new _cout);
  addPrototype(new drawCircle);
  addPrototype(new intRGBAColor);
  addPrototype(new ofPointCon);
  addPrototype(new unaryOp<int, string>("intToString", "int","int","string","string",&intToString));
  addPrototype(new unaryOp<float, int>("floatToUint8", "float","float","int","int",&scaleInt));
  addPrototype(new floatTL(&timeline, "floatTL"));
  addPrototype(new colorTL(&timeline, "colorTL"));
  //UI init stuff
  selectedDuct = NULL;
  selectedOp=NULL;
    
  int i = 0;
  for (map<string, ofxOperator*>::iterator it = opPrototypes.begin(); it!=opPrototypes.end(); it++) {
    ofxMSAInteractiveObjectWithDelegate* temp = new ofxMSAInteractiveObjectWithDelegate();
    temp->setLabel(it->first);
    temp->setPosAndSize(-500, (ofGetHeight()-30) - (i*30), 80, 20);
    temp->setDelegate(&newOpHandler);
    protoButtons.push_back(temp);
    i++;
  }
    
    
  //loading file has to happen after you load proto objects.  
  openJSON(ofToDataPath("example.json"));
    
}

void testApp::openJSON(string path){
  ifstream tempfile;
  tempfile.open(path.c_str());
    
  Json::Value root;
  Json::Value jsnodes;
  Json::Value jsedges;
  tempfile >> root;
  jsnodes = root["nodes"];
  jsedges = root["edges"];
    
  //debug
  cout<< "nodes: " << jsnodes;
  cout<< "edges: " << jsedges;
    
  cout<< "nodes-size: " <<jsnodes.size() <<endl;
  for (int i = 0; i< jsnodes.size(); i++){
    cout<< "node: " << jsnodes[i]["type"].asString() <<endl;
    cout<< "node x: " << jsnodes[i].get("posx", 40) <<endl;
    addOperatorFromJson(  
			jsnodes[i]["type"].asString(), 
			ofPoint(
				jsnodes[i].get("posx", 50).asInt(),
				jsnodes[i].get("posy", 50).asInt()
				),
			jsnodes[i].get("uid",-1).asInt(),
			jsnodes[i]["opdata"]
			  );        
  }
  cout << "size of edges:" <<jsedges.size() <<endl;
  for (int i = 0; i< jsedges.size(); i++) {
    if (jsedges[i] != Json::nullValue) {
            
        
      addDuctFromJson(  
		      jsedges[i]["from"].asInt(),
		      jsedges[i]["out"].asString(),
		      jsedges[i]["to"].asInt(),
		      jsedges[i]["in"].asString());

    }
  }
    
  tempfile.close();         
}

void testApp::saveJSON(string path){
  ofstream tempfile;
  tempfile.open(path.c_str());
  map<int, ofxOperator*>::iterator it;
  Json::Value jsnodes;
  Json::Value jsedges;
  int i = 0;
  int j = 0;
  cout << "size" <<opGraph.size()<<endl;
  for(it = opGraph.begin(); it!= opGraph.end(); it++){
    ofxOperator * tempOp = it->second;
    if(tempOp != NULL){
      jsnodes[i]["uid"] = tempOp->uid;
      jsnodes[i]["type"] = tempOp->operatorType;
      jsnodes[i]["posx"] = tempOp->uiElement->x;
      jsnodes[i]["posy"] = tempOp->uiElement->y;
      jsnodes[i]["opdata"] = tempOp->saveJSON();
      if (tempOp->uiElement!=NULL){
	map<string,ofxDuctUI*>::iterator itb;

	for (itb = tempOp->uiElement->inputs.begin();itb != tempOp->uiElement->inputs.end(); itb++){

	  if (itb->second != NULL){
	    if (itb->second->inputLink != NULL){
	      if (itb->second->inputLink->op != NULL){
		jsedges[j]["from"] = itb->second->inputLink->op->uid;
		jsedges[j]["out"] = itb->second->inputLink->getLabel();
		jsedges[j]["to"] = tempOp->uid;
		jsedges[j]["in"] = itb->first;
		j++; //increment only on success, otherwise get null json values. 
                             
	      } else {cout << "op null: "<< itb->first <<endl;}
	    }else{ cout <<"inputlink null: "<< itb->first<<  endl;}
                    

	  } else { cout << "itb->second issues" <<endl;}
	  cout << "j at" <<j<<endl;
	}
      } else { cout << "tempOp->uiElement is null aparently." << endl;}
    } else { cout << "tempOp is null for uid:"  << it->first <<endl;}
    i++;    
  }
  Json::Value root;
  root["nodes"] = jsnodes;
  root["edges"] = jsedges;
    
  tempfile << root;
  tempfile.close();
                
}

void testApp::update() {
  px=mouseX;
  py=mouseY;
}

void testApp::draw() {
  ofSetWindowTitle(ofToString(ofGetFrameRate()));
  ofFill();
  ofBackground(0, 0, 0);
  //  ofRect(50, 50, 300, 300);    



   for(list<ofxOperator*>::iterator it = runQueue.begin(); it!= runQueue.end(); it++){
    (*it)->run();
   }

   ///UI STUFF AT THE END AS OVERLAY    
  timeline.draw();
   drawCursor(cursor);
   if(selectedDuct != NULL){
    ofPushStyle();
    ofFill();
    ofSetColor(255, 255, 255,70);
    ofRect(*selectedDuct);
    ofPopStyle();
  }
}


void testApp::addPrototype(ofxOperator* newProto){
  opPrototypes.insert(make_pair(newProto->operatorType, newProto));
}

int testApp::generateUID(){
  if (freeUIDs.size()>0){
    int temp = freeUIDs.front();
    freeUIDs.pop_front();
    return temp;
  }else{
    return uidCounter++;
  }
}

void testApp::newOpToGraph(ofxOperator* theOp, int uid){
    
  theOp->uid = uid;
  opGraph.insert(make_pair(uid, theOp));  //error prone, but unique for now. 
  map<string, int> tempNode;
  dag[uid] = make_pair(0, tempNode); //creates an empty dag map entry.     
  cout << "new op with uid: " <<  uid <<endl;
    
}

void testApp::removeOp(ofxOperator* theOp){
  //disconnect ducts
  cout << "op:id" << theOp->uid << endl;
  if (dag.find(theOp->uid) != dag.end()){
    //iterate over removal candidates inputs first, then iterate over outlets connected  
    cout << "found op in dag" <<endl;
    map<string, int>* inputs = &(dag.find(theOp->uid)->second.second);
    // <inputname, uid> from dag def.
    while (inputs->size() != 0){
      cout << "in while loop" << endl;
      int uhoh = inputs->size(); 
      removeDuctFromDAG(inputs->begin()->second, theOp->uid, inputs->begin()->first);
      if  (uhoh == inputs->size()){
	cout << "removal not working" <<endl;
	break; //no infinite loop please.
      } else { cout << "removal worked!" << endl; }
      //dont need to clear ui elements on op to be deleted, done within op destructor.    
    } 

//find the outputs.  
    pair<multimap<int, pair<int, string> >::iterator, multimap<int, pair<int, string> >::iterator> outRange;
    for(outRange = outputLinks.equal_range(theOp->uid);outRange.first != outRange.second; outRange.first++){
      int from = outRange.first->first;
      int to = outRange.first->second.first;
      string inputname = outRange.first->second.second;
      removeDuctFromDAG(from, to, inputname);
            if (opGraph.find(to)!=opGraph.end()){ //quick fix. is opgraph clearing properly?

      opGraph.find(to)->second->clearInput(inputname);
      opGraph.find(to)->second->uiElement->inputs.find(inputname)->second->inputLink = NULL;
        }
    }
    outputLinks.erase(theOp->uid); //this kills all of them, thanks multimap.
    opGraph.erase(theOp->uid);
    dag.erase(theOp->uid);
    freeUIDs.push_front(theOp->uid);
    runQueue.remove(theOp);
    //destroy    
    delete theOp->uiElement;
    delete theOp;

    selectedDuct = NULL;
    selectedOp = NULL;

    cout << "deleted op" << endl;
    }
}

void testApp::addOperatorFromUI(string opType,ofPoint location){
  //add op to graph
  //bind the ui object
  map<string, ofxOperator*>::iterator tempProto = opPrototypes.find(opType);
  if (tempProto != opPrototypes.end()){
    ofxOperator * tempOp = tempProto->second->clone();
        
    int tempuid = generateUID();    
    newOpToGraph(tempOp, tempuid); 
    
    //    cout << "this: " << this<<endl;
    ofxOperatorUI * tempUi = new ofxOperatorUI;
    tempUi->setDelegate(this);
    tempUi->setPosAndSize(location.x, location.y, 100, 20);
    tempUi->op = tempOp;
    tempOp->uiElement = tempUi;
    tempUi->setup();
    tempUi->populate(&EPhandler); 
  }
  hideProtoMenu();
}

void testApp::addDuctFromJson(int fromUid, string fromOutputName, int toUid, string toInputName){
  map<int, ofxOperator*>::iterator tempFrom = opGraph.find(fromUid);
  map<int, ofxOperator*>::iterator tempTo = opGraph.find(toUid);
    
  if ((tempFrom != opGraph.end()) && (tempTo != opGraph.end())) {
        
    if (typeCheckDuct(
		      tempFrom->second,
		      fromOutputName,
		      tempTo->second,
		      toInputName)
	&&
	tsortCheckDuct(fromUid,
		       toUid,
		       toInputName)
	){                        
      tempTo->second->bindInput(toInputName, tempFrom->second->getOutput(fromOutputName));
      tempTo->second->uiElement->inputs.find(toInputName)->second->inputLink =
	tempFrom->second->uiElement->outputs.find(fromOutputName)->second;
      cout << "json duct typecheck passed " << endl;
      outputLinks.insert(make_pair(fromUid, make_pair(toUid, toInputName)));
    }        
  }        
}
void testApp::addOperatorFromJson(string opType, ofPoint location, int uid, Json::Value opData){
  //add op to graph
  //bind the ui object
  map<string, ofxOperator*>::iterator tempProto = opPrototypes.find(opType);
  if (tempProto != opPrototypes.end()){

    ofxOperator * tempOp = tempProto->second->clone();
    tempOp->loadJSON(opData);
    newOpToGraph(tempOp, uid); 
    uidCounter = uid + 1;    

    cout << "this: " << this<<endl;
    ofxOperatorUI * tempUi = new ofxOperatorUI;
    tempUi->setDelegate(this);
    tempUi->setPosAndSize(location.x, location.y, 100, 20);
    tempUi->op = tempOp;
    tempOp->uiElement = tempUi;
    tempUi->setup();
    tempUi->populate(&EPhandler);
  }
}

void testApp::addDuctFromUI(list<ofxDuctUI*>* candidates){
  if (candidates->size()>=2) {
    for(list<ofxDuctUI*>::iterator it = candidates->begin();it!=candidates->end();it++){
      it++; //start from two
      cout << "it null:" << (it==candidates->end()) <<endl;
      cout << "front op null:" <<((candidates->front()->op)==NULL) << endl;
      cout << "back op null:" <<((candidates->back()->op)==NULL) << endl;
      cout << "front op label:" <<candidates->front()->getLabel() << endl;
      cout << "back op label:" <<candidates->back()->getLabel() << endl;
      cout << "front type:" <<candidates->front()->op->outputTypes.find(candidates->front()->getLabel())->second << endl;
      cout << "back size:" <<candidates->back()->op->inputTypes.size() << endl;
      cout << "front size:" <<candidates->front()->op->inputTypes.size() << endl;
      cout << "from uid: " <<candidates->front()->op->uid;
      cout << "to uid: " <<candidates->back()->op->uid;
      if ((typeCheckDuct(candidates->front()->op, 
			 candidates->front()->getLabel(), 
			 (*it)->op, 
			 (*it)->getLabel()))
	  &&
	  tsortCheckDuct(candidates->front()->op->uid,
			 candidates->back()->op->uid,
			 candidates->back()->getLabel())
	  ) {

	cout << "typecheck+tsort passed!" <<endl;
	(*it)->op->bindInput(
			     (*it)->getLabel(),
			     candidates->front()->op->getOutput(candidates->front()->getLabel()));
	(*it)->inputLink = candidates->front();
	int fromUid = candidates->front()->op->uid;
	int toUid = (*it)->op->uid;
	outputLinks.insert(make_pair(fromUid, make_pair(toUid, (*it)->getLabel())));
	cout << "input link inserted in UI:" <<( (*it)-> inputLink != NULL) << endl;
	if((*it)->inputLink != NULL){
	  cout << "inputlink y:" <<  (*it)->inputLink->y << endl;
	}
      }
    }        
  }
}
               
bool testApp::typeCheckDuct(ofxOperator* from, string outputName, ofxOperator* to, string inputName){
  cout << "from: " << outputName <<endl;
  cout << "outputTypes size: " <<from->outputTypes.size() <<endl;
  cout << "from type: "<< from->outputTypes.find(outputName)->second << endl;
  cout << "to: " <<inputName << endl;
  cout << "to type: " <<to->inputTypes.find(inputName)->second <<endl;
       
  return ((from->outputTypes.find(outputName)->second.compare(to->inputTypes.find(inputName)->second)) == 0);
    
}

bool testApp::tsortCheckDuct(int from, int to, string toInput){
  //adapted from the tsort algorithm in Knuth, Donald E. (1997). The Art of Computer Programming. 1 (3rd ed.). pp. 261–268 
  //sorry about all the dragons, will refactor this to be prettier soon. 

  map<int, pair<int, map<string, int> > > dagtemp = dag; //short for Directed Acyclic Graph
  //uid,   num out,   inputName, uid of linked 
  cout <<"dagtemp size:"<<dagtemp.size()<< endl;
  map<int, pair<int, map<string, int> > >:: iterator it;
  for (it = dagtemp.begin(); it != dagtemp.end(); it++){
    cout<< "node UID:"<<it->first <<endl;   
  }
  if ((dagtemp.find(from) != dagtemp.end())&&  
      (dagtemp.find(to) != dagtemp.end())
      ) {
    dagtemp.find(from)->second.first++; //increment number of outputs on from
    dagtemp.find(to)->second.second[toInput] = from; //attach the link to the "from" node on "to" 
  } else { 
        
    if(dagtemp.find(from) == dagtemp.end()){cout << "oops from" <<from <<endl;};
    if(dagtemp.find(to) == dagtemp.end()){cout << "oops to" <<to<<endl;};
    return false; 
  }
  //find zeros
  //map<int, pair<int, map<string, int> > >::iterator it;
  list<int> zeros;
  list<int> sorted;
  for (it = dagtemp.begin(); it!=dagtemp.end(); it++){
    if (it->second.first == 0){
      zeros.push_back(it->first);
    }

  }
  map<int, pair<int, map<string, int> > > dagSwap = dagtemp;

    
  //decrement links, move to output
  list<int>::iterator itb;
  for(itb = zeros.begin();itb!= zeros.end(); itb++){
    map<string, int>::iterator itc;
    for (itc = dagtemp.find(*itb)->second.second.begin(); itc != dagtemp.find(*itb)->second.second.end(); itc++){
      dagtemp.find(itc->second)->second.first--; 
      if (dagtemp.find(itc->second)->second.first == 0){
	zeros.push_back(itc->second);
      }
            
    }
    sorted.push_front(*itb);        
  }
  if (sorted.size() == dagtemp.size()){
    dag = dagSwap; 

    runQueue.clear();
    for (list<int>::iterator itd = sorted.begin(); itd != sorted.end(); itd++){
      runQueue.push_back(opGraph.find(*itd)->second);
    }
    cout<< "tsort success! size:" <<sorted.size()<<endl;
    return TRUE;
  }else{
    cout<< "tsort failure. oops?" <<endl;
    return FALSE;
  }
}


void testApp::removeDuctFromDAG(int from, int to, string toInput){        
  if (dag.find(from) != dag.end())dag.find(from)->second.first--;
  //reduce the output count
  if (dag.find(to) != dag.end())dag.find(to)->second.second.erase(toInput);
  // remove the input link.
}

void testApp::removeDuctFromOutputLinks(int from, int to, string toInput){
  pair<multimap<int, pair<int, string> >::iterator, multimap<int, pair<int, string> >::iterator> outRange;
  for (outRange = outputLinks.equal_range(from); outRange.first != outRange.second; outRange.first++){
    if((outRange.first->second.first == to)&&(outRange.first->second.second.compare(toInput) != 0)){
	outputLinks.erase(outRange.first);
	break; //only should be one, so stop iterating. 
    }
  }
}
  
void testApp::keyPressed(int key){
  if(keyboardFocus){
  switch (key) {
  case OF_KEY_BACKSPACE:
    if (selectedDuct!=NULL) {
      if ((selectedDuct->inputLink!=NULL)&&
	  (selectedDuct->inputLink->op!=NULL)&&
	  (selectedDuct->op!=NULL)
	  ) { 
	int from = selectedDuct->inputLink->op->uid; 
	int to = selectedDuct->op->uid; 
	removeDuctFromDAG(from, to, selectedDuct->getLabel());
	removeDuctFromOutputLinks(from,to, selectedDuct->getLabel());
      }
      selectedDuct->op->clearInput(selectedDuct->getLabel());
      selectedDuct->inputLink = NULL;
    }else
    if (selectedOp!=NULL) {
      removeOp(selectedOp->op);
    }

    break;
  case 'n':
    showProtoMenu();
    break;
  case ' ':
    hideProtoMenu();
    break;
  case 's':
    saveJSON(ofToDataPath("example.json"));
    break;
  case 'd':
    for(list<ofxOperator*>::iterator it = runQueue.begin(); it!= runQueue.end(); it++ ){
      cout << (*it)->uid << ": " <<(*it)->operatorType << endl;
    }
    for(map<int, pair<int, map<string, int> > >::iterator itb = dag.begin(); itb!= dag.end(); itb++){
      cout << "dag node: " << itb->first << endl;
    }
    break;

  default:
    break;
  }
  }
}

void testApp::showProtoMenu(){
  if (protoButtons.size()!=0){
    int i = 0;
    for (list<ofxMSAInteractiveObjectWithDelegate*>::iterator it = protoButtons.begin(); it!= protoButtons.end(); it++) {
      (*it)->setPosAndSize(ofGetWidth()-120, (ofGetHeight()-40) - (i*30), 80, 20);
      i++;
      //      (*it)->x = ofGetWidth()-100;
    }                    
  }
}
void testApp::hideProtoMenu(){
  if (protoButtons.size()!=0){
    for (list<ofxMSAInteractiveObjectWithDelegate*>::iterator it = protoButtons.begin(); it!= protoButtons.end(); it++) {
      (*it)->x = -500;
    }                    
  }
}

void testApp::mouseReleased(int x, int y, int button){
  for (list<ofxMSAInteractiveObject*>::iterator it = dragQueue.begin(); it != dragQueue.end(); it++) {
    (**it).setPos((**it).x+(x-px), (**it).y+(y-py));
  }
  dragQueue.clear();
  cursor=ofPoint(x,y);
}


void testApp::mousePressed(int x, int y, int button){

  dragFrom = ofPoint(x,y);
    
}
void testApp::mouseDragged(int x, int y, int button){
  for (list<ofxMSAInteractiveObject*>::iterator it = dragQueue.begin(); it != dragQueue.end(); it++) {
    (**it).setPos((**it).x+(x-px), (**it).y+(y-py));
  }

}

void testApp::dragEvent(ofDragInfo dragInfo){
}

void testApp::gotMessage(ofMessage msg){
  if (msg.message.compare("textfieldIsActive")==0){ 
      keyboardFocus=false;
    }
  if (msg.message.compare("textfieldIsInactive")==0){
      keyboardFocus=true;
    }
}

void testApp::drawCursor(ofPoint thePoint){
  ofSetColor(255, 255, 255);
  ofLine(thePoint.x-20, thePoint.y, thePoint.x-15, thePoint.y);
  ofLine(thePoint.x+20, thePoint.y, thePoint.x+15, thePoint.y);
  ofLine(thePoint.x, thePoint.y-20, thePoint.x, thePoint.y-15);
  ofLine(thePoint.x, thePoint.y+20, thePoint.x, thePoint.y+15);    
}

void testApp::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){
}
void testApp::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){}
void testApp::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){
  dragQueue.push_back(object);
  selectedOp = dynamic_cast<ofxOperatorUI*>(object);
  selectedDuct = NULL;
}	
void testApp::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button)	{}
void testApp::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){}
void endPointEventHandler::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){}
void endPointEventHandler::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){}
void endPointEventHandler::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){

  ofxDuctUI* dobject = dynamic_cast<ofxDuctUI*>(object); 
  if (dobject==NULL) {
    cout<< "dobject null" <<endl;
  }
  if ((controller != NULL)&&(dobject != NULL)) {
    controller->selectedDuct = dobject;
    controller->selectedOp = NULL;
    if ((connectionCandidates.size()==0)&&(!(dobject->isInput))) {
      connectionCandidates.push_back(dobject);
      cout<< "first click: " <<endl;
    }
    if ((connectionCandidates.size()!=0)&&((dobject->isInput))) {
      cout<< "second click" <<endl;
      connectionCandidates.push_back(dobject);
      cout << "cc size:" << connectionCandidates.size() <<endl;
      cout << "cc ref:" << &connectionCandidates << endl;
      controller->addDuctFromUI(&connectionCandidates);
      connectionCandidates.clear();
    }        
  }    
}
void endPointEventHandler::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button){}
void endPointEventHandler::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){}


void newOperatorEventHandler::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){}
void newOperatorEventHandler::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){}
void newOperatorEventHandler::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){
  ofxMSAInteractiveObjectWithDelegate * temp = dynamic_cast<ofxMSAInteractiveObjectWithDelegate*>(object);
  if (temp!=NULL) {
    controller->addOperatorFromUI(temp->getLabel(),controller->cursor);
  }

}
void newOperatorEventHandler::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button){}
void newOperatorEventHandler::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){}


void ofxOperator::bindInput(string inputName, ofxDuctBase* duct){
    
}
ofxDuctBase* ofxOperator::getOutput(string outputName){
    
}
void ofxOperator::clearInput(string inputName){
  bindInput(inputName, NULL);
    
}
list<pair<string,string> > ofxOperator::revealFreeInputs(){
    
}

void ofxOperator::run(){
    
}


void ofxOperatorUI::draw(){
  ofPushStyle();
    
  if(isMouseDown()){
    ofSetColor(255,0,0);
    ofFill();
  }
  else if(isMouseOver()){
    ofSetColor(0,0,255,120);
        ofFill();
  }
  else{
    ofSetColor(255,255,255,50);
    ofFill();
  }
  ofRect(*this);
  ofSetColor(255);
  if(getLabel() != ""){
    ofDrawBitmapString(getLabel(), x+10, y+15);
  }

}

inline void bezConnect(ofPoint from, ofPoint to, float height){
    ofNoFill();  
      ofSetLineWidth(height);
      ofBezier(from.x, from.y+(height/2), ((to.x-from.x)*.5)+from.x, from.y+(height/2), ((to.x-from.x)*.5)+from.x, to.y+(height/2), to.x, to.y+(height/2));
}

void ofxDuctUI::draw(){
  this->ofxMSAInteractiveObjectWithDelegate::draw(); 
  if (inputLink != NULL){
    ofPushStyle();
    ofFill();
    ofSetColor(255, 255, 255,70);
    bezConnect(ofPoint(this->x, this->y), ofPoint(this->inputLink->x+this->inputLink->width, this->inputLink->y),this->height);
    ofPopStyle();
  }
}

void ofxOperatorUI::populate(ofxMSAInteractiveObjectDelegate* delegate){
    
  if (op!=NULL){
    setLabel(op->operatorType);
    int i = 0;
    for(map<string,string>::iterator it = op->inputTypes.begin(); it != op->inputTypes.end(); it++){
      ofxDuctUI * temp = new ofxDuctUI();
      temp->isInput=TRUE;
      temp->op = op;
      temp->inputLink =NULL;
      temp->setDelegate(delegate);
      temp->setPosAndSize(this->x, this->y+(i*30)+30, 20, 20);
      temp->setLabel(it->first);
      temp->setup();
      children.push_back(temp);
      inputs.insert(make_pair(it->first, temp));
      i++;
    }
    i = 0;
    for(map<string,string>::iterator it = op->outputTypes.begin(); it != op->outputTypes.end(); it++){            
      ofxDuctUI * temp = new ofxDuctUI();
      temp->isInput=FALSE;
      temp->op = op;
      temp->inputLink =NULL;
      temp->setDelegate(delegate);
      temp->setPosAndSize(this->x+this->width-40, this->y+(i*30)+30, 20, 20);
      temp->setLabel(it->first);
      temp->setup();
      children.push_back(temp);
      outputs.insert(make_pair(it->first, temp));
      i++;
    }
    op->uiAvailable();
  }
};

