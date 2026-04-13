#include "malevrovich_prof/instrumentation.h"

#include "malevrovich_prof/profiler.h"

extern "C" {

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void* this_fn,
                                                                      void* call_site) {
  (void)this_fn;
  (void)call_site;
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void* this_fn,
                                                                     void* call_site) {
  (void)this_fn;
  (void)call_site;
}

}  // extern "C"
