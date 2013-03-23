ofxDuct

ofxDuct is a proof of concept for a visual programming UI for openframeworks libraries and addons. 
ofxDuct follows the usual interface style for visual programming, the node graph, but uses different execution semantics (!!).
ofxDuct is neither event based nor object oriented. Nodes represent functors, and the edges between the nodes indicate the data dependencies of these functors. These functors can represent any c++ function or functor, and the edges are strongly typed, and can represent any non abstract c++ type (and yes, that includes function pointers!). 
Execution of the graph is strict, executing each functor exactly once per cycle in order of data dependency between the nodes.
serialization is done in json, and functors can have their own custom data fields for saving state between sessions. 

I have implemented only a few functors  here to show a working system, but I will keep adding to these myself, and gladly welcome any contributions. Adding stateless functions is actually super simple (checkout the unaryOp<> examples in the addPrototype section at the top of  testApp.cpp to see what I mean).

building for current version of openFrameworks:

get ofxTimeline, and its dependencies,  from  https://github.com/YCAMInterlab/ofxTimeline

checkout my branch of ofxMSAInteractiveObject from https://github.com/cwright/ofxMSAInteractiveObject.git

keyboard reference:
s: save
space: startup the timeline
n: new functor
d: show execution order at stdout



