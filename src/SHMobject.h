#if !defined BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif
#ifndef SHMOBJECT_H
#define SHMOBJECT_H

#include <node.h>
#include <string.h>

extern "C" {
	//SHARED MEMORY
	char *shm_err();
	void shm_setDbg( int on );
	
	void shmop_init();
	void shmop_deinit();
	int shmop_open (int key, char* flags, int mode, int size);
	unsigned char* shmop_read (int shmid, int start, int count);
	void shmop_close (int shmid);
	int shmop_size (int shmid);
	int shmop_write (int shmid, unsigned char * data, int offset, int data_len);
	int shmop_delete (int shmid);
}

class SHMobject : public node::ObjectWrap {
 public:
  static void Init();
  static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
  double Val() const { return val_; }

 private:
  SHMobject();
  ~SHMobject();

  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  double val_;
};

#endif
