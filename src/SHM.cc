#if !defined BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include "node_buffer.h"
#include "SHMobject.h"

using namespace node;
using namespace v8;


Handle<Value> GetErrMsg(const Arguments& args) {
	HandleScope scope;
	int activateDebug = args[0]->Int32Value();	//IntegerValue();
	shm_setDbg(activateDebug);
	Local<Value> b =  Encode(shm_err(), 1024, BINARY);
	return scope.Close(b);
}

//API:	
//	int shmid  .... int shmop_open (int key, char* flags, int mode, int size)	
//	unsigned char*  shmop_read (int shmid, int start, int length)
//	int  shmop_close (int shmid)
//	int  shmop_size (int shmid)
//	int  shmop_write (int shmid, char * data, int offset, int data_len)
//	int shmop_delete (int shmid)
//

Handle<Value> ShmOpen(const Arguments& args) {
//	int shmid  .... int shmop_open (int key, char* flags, int mode, int size)	
  HandleScope scope;
  char *flags = NULL;
  int key = args[0]->Int32Value();	//IntegerValue();
  String::Utf8Value sflags(args[1]->ToString());;
  flags = new char[(sflags.length()+1)];
  memcpy(flags,*sflags,sflags.length()+1);
  int mode = args[2]->Int32Value();	//IntegerValue();
  int size = args[3]->Int32Value();	//IntegerValue();

  int retval = shmop_open (key, flags, mode, size);
  return scope.Close(Number::New(retval));
}

Handle<Value> ShmRead(const Arguments& args) {
//	unsigned char*  shmop_read (int shmid, int start, int length)
  HandleScope scope;
  char *sdata;
  int shmid = args[0]->Int32Value();	//IntegerValue();
  int start = args[1]->Int32Value();	//IntegerValue();
  int length = args[2]->Int32Value();	//IntegerValue();
  
  sdata = (char *)shmop_read (shmid, start, length);
  if (sdata > 0) {
//	  Local<Value> b =  Encode(sdata, length, BINARY);
//	  return scope.Close(b);

	Buffer *slowBuffer = Buffer::New(length);
	memcpy(Buffer::Data(slowBuffer), sdata, length);

	Local<Object> globalObj = Context::GetCurrent()->Global();
	Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
	Handle<Value> constructorArgs[3] = { slowBuffer->handle_, v8::Integer::New(length), v8::Integer::New(0) };
	Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
	return scope.Close(actualBuffer);

  } else {
	Local<Value> e = Exception::Error(String::NewSymbol("SHM read error"));
	return scope.Close(e);
  }
}

Handle<Value> ShmClose(const Arguments& args) {
//	int  shmop_close (int shmid)
  HandleScope scope;
  int shmid = args[0]->Int32Value();	//IntegerValue();
  
  shmop_close (shmid);
  return scope.Close(Number::New(1));
}

Handle<Value> ShmSize(const Arguments& args) {
//	int  shmop_size (int shmid)
  HandleScope scope;
  int shmid = args[0]->Int32Value();	//IntegerValue();
  
  int retval = shmop_size (shmid);
  return scope.Close(Number::New(retval));
}

Handle<Value> ShmWrite(const Arguments& args) {
//	int  shmop_write (int shmid, char * data, int offset, int data_len)
  HandleScope scope;
  int shmid = args[0]->Int32Value();	//IntegerValue();
  //Local<String> sdata = args[1]->ToString();
    Local<Value> buffer_v = args[1];
    if (!Buffer::HasInstance(buffer_v)) {
      return ThrowException(Exception::TypeError(
            String::New("Argument should be a buffer")));
    }
    Local<Object> buffer_obj = buffer_v->ToObject();
    char *buffer_data = Buffer::Data(buffer_obj);
//    size_t buffer_len = Buffer::Length(buffer_obj);
  int offset = args[2]->Int32Value();	//IntegerValue();
  int data_len = args[3]->Int32Value();	//IntegerValue();
  
  int retval = shmop_write (shmid, (unsigned char *)buffer_data, offset, data_len);
  return scope.Close(Number::New(retval));
}

Handle<Value> ShmDelete(const Arguments& args) {
//	int shmop_delete (int shmid)
  HandleScope scope;
  int shmid = args[0]->Int32Value();	//IntegerValue();
  
  int retval = shmop_delete (shmid);
  return scope.Close(Number::New(retval));
}


void Initialize(Handle<Object> target) {
  SHMobject::Init();

	  
  target->Set(String::NewSymbol("getErrMsg"),
      FunctionTemplate::New(GetErrMsg)->GetFunction());
	  
  target->Set(String::NewSymbol("openSHM"),
      FunctionTemplate::New(ShmOpen)->GetFunction());
  target->Set(String::NewSymbol("readSHM"),
      FunctionTemplate::New(ShmRead)->GetFunction());
  target->Set(String::NewSymbol("closeSHM"),
      FunctionTemplate::New(ShmClose)->GetFunction());
  target->Set(String::NewSymbol("sizeSHM"),
      FunctionTemplate::New(ShmSize)->GetFunction());
  target->Set(String::NewSymbol("writeSHM"),
      FunctionTemplate::New(ShmWrite)->GetFunction());
  target->Set(String::NewSymbol("deleteSHM"),
      FunctionTemplate::New(ShmDelete)->GetFunction());
}

NODE_MODULE(shm, Initialize)	//name of module in NODE !!! =  "shm"
