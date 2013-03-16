#if !defined BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include "SHMobject.h"

using namespace v8;

SHMobject::SHMobject() {};
SHMobject::~SHMobject() {};

Persistent<Function> SHMobject::constructor;

void SHMobject::Init() {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("SHMobject"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> SHMobject::New(const Arguments& args) {
  HandleScope scope;

  SHMobject* obj = new SHMobject();
  obj->val_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> SHMobject::NewInstance(const Arguments& args) {
  HandleScope scope;

  const unsigned argc = 1;
  Handle<Value> argv[argc] = { args[0] };
  Local<Object> instance = constructor->NewInstance(argc, argv);

  return scope.Close(instance);
}
