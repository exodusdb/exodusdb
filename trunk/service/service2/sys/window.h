#ifndef EXODUSDLFUNC_WINDOW_H
#define EXODUSDLFUNC_WINDOW_H

#include <exodus/xfunctorf1.h>
ExodusFunctorF1<in> security("window","security");

#include <exodus/xfunctorf2.h>
ExodusFunctorF2<in,in> security2("window","security2");

ExodusFunctorF2<in,in> security3("window","security3");

ExodusFunctorF1<in> invalidq("window","invalidq");

#include <exodus/xfunctorf0.h>
ExodusFunctorF0<int> invalid("window","invalid");

ExodusFunctorF1<in> invalid("window","invalid");

ExodusFunctorF1<in> invalid2("window","invalid2");

ExodusFunctorF1<in> note3("window","note3");
#endif
