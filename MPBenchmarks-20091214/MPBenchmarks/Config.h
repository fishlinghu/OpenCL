// Config flags, common includes, and utils
// (c) EB Nov 2009

#ifndef Config_h
#define Config_h

// Configuration. The flags may be defined on the compiler command line,
// and in this case we don't change them here.

// Use Qt interface?
#ifndef CONFIG_USE_QT
#define CONFIG_USE_QT 1
#endif

// Use MPIR library to cross-check results?
#ifndef CONFIG_USE_MPIR
#define CONFIG_USE_MPIR 0
#endif

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#endif

// Minimal run time required to have a valid measurement (s)
const double MIN_RUNNING_TIME = 0.2;

// CPU memory alloc alignment (bytes)
const int ALLOC_ALIGN = 4096;

#include <malloc.h>
#include <math.h>
#include <string>

// Windows specific
#ifdef WIN32
#include <emmintrin.h>
#include <windows.h>
inline double getT()
{
  LARGE_INTEGER c,f;
  QueryPerformanceFrequency(&f);
  QueryPerformanceCounter(&c);
  return (double)c.QuadPart/(double)f.QuadPart;
}
#endif

// Linux specific
#ifdef Linux
#include <sys/time.h>
#include <pthread.h>
inline void * _aligned_malloc(size_t sz,size_t a) { return memalign(a,sz); }
inline void _aligned_free(void * x) { free(x); }
#define _snprintf snprintf
inline double getT()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  return (double)tv.tv_sec+1.0e-6*(double)tv.tv_usec;
}
#endif

// Common
#if CONFIG_USE_MPIR
#include <mpir.h>
#endif
#include "BealtoOpenCL.h"

// Text logger
class Logger
{
public:
  virtual ~Logger() { }
  virtual void append(const char * s) { printf("%s\n",s); fflush(stdout); }
  void append(const std::string & s) { append(s.c_str()); }
  virtual void appendTitle(const char * s) { printf("\n*** %s\n\n",s); fflush(stdout); }
  void appendTitle(const std::string & s) { appendTitle(s.c_str()); }
};

#if CONFIG_USE_QT
#include <QtGui/QTextEdit>
#include <QtGui/QApplication>
// Qt QTextEdit logger
class QtLogger : public Logger
{
public:
  QtLogger(QTextEdit * e) : mE(e) { }
  void append(const char * s)
  {
    QTextCharFormat f;
    f.setFontFamily("fixed");
    mE->setCurrentCharFormat(f);
    mE->append(s);
    qApp->processEvents();
  }
  void appendTitle(const char * s)
  {
    QTextCharFormat f;
    f.setFontWeight(QFont::Bold);
    mE->setCurrentCharFormat(f);
    mE->append(s);
    qApp->processEvents();
  }
private:
  QTextEdit * mE;
};
#endif

// Return user friendly size from LOG (size = 1<<LOG) in S.
inline void getUserSize(int log,std::string & s)
{
  if (log < 0 || log >= 40) { s.assign("???"); return; }
  char aux[200];
  if (log < 10) _snprintf(aux,200,"%d B",1<<log);
  else if (log < 20) _snprintf(aux,200,"%d KiB",1<<(log-10));
  else if (log < 30) _snprintf(aux,200,"%d MiB",1<<(log-20));
  else if (log < 40) _snprintf(aux,200,"%d GiB",1<<(log-30));
  s.assign(aux);
}

#ifdef Linux
typedef void * (*ThreadProc)(void *);
#define thread_return_t void *
#endif
#ifdef WIN32
typedef LPTHREAD_START_ROUTINE ThreadProc;
#define thread_return_t DWORD WINAPI
#endif

// Run one CPU thread calling F for each parameter in the PARAMS array.
// Return the real time of execution, or -1 on error.
template <class P> double runCPUThreads(std::vector<P> & params,ThreadProc f)
{
  int nt = (int)params.size(); // thread count
  if (nt <= 0) return -1; // invalid
  double t0 = getT();
  bool ok = true;
#ifdef Linux
  std::vector<pthread_t> threads(nt,0);
  // start all threads
  for (int i=0;i<nt;i++)
    {
      int s = pthread_create(&(threads[i]),0,f,&(params[i]));
      if (s != 0) { ok = false; threads[i] = 0; }
    }
  // join all threads
  for (int i=0;i<nt;i++)
    {
      if (threads[i] == 0) continue; // creation failed
      int s = pthread_join(threads[i],0);
      if (s != 0) ok = false;
    }
#endif
#ifdef WIN32
  std::vector<HANDLE> threads(nt,0);
  // start all threads
  for (int i=0;i<nt;i++)
    {
      threads[i] = CreateThread(0,0,f,&(params[i]),0,0);
      if (threads[i] == 0) ok = false;
    }
  // wait for all threads to terminate
  WaitForMultipleObjects(nt,&(threads[0]),TRUE,INFINITE);
  // delete threads
  for (int i=0;i<nt;i++) CloseHandle(threads[i]);
#endif
  if (!ok) return -1; // error
  double t = getT() - t0;
  return t;
}

#endif // Config_h
