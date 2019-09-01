include_directories(AFTER "include/")

set(LIBCOMMON_SRC
  libcommon/expr.c
  libcommon/rotate.c
  libcommon/sampling.c
  libcommon/scale.c
  libcommon/transform.c
  libcommon/translate.c
  libcommon/vecmath.c
  libcommon/xform.c
)

set(LIBCOMMON_HEADER
  include/libcommon/color.h
  include/libcommon/common.h
  include/libcommon/error.h
  include/libcommon/expr.h
  include/libcommon/ray.h
  include/libcommon/rotate.h
  include/libcommon/sampling.h
  include/libcommon/scale.h
  include/libcommon/transform.h
  include/libcommon/translate.h
  include/libcommon/vector.h
  include/libcommon/xform.h
)

set(LIBIMAGE_SRC
  libimage/image.c
)

set(LIBIMAGE_HEADER
  include/libimage/image.c
)

set(LIBOBJ_SRC
  libobj/blob.c
  libobj/bounds.c
  libobj/box.c
  libobj/cone.c
  libobj/csg.c
  libobj/cylinder.c
  libobj/disc.c
  libobj/geom.c
  libobj/grid.c
  libobj/hf.c
  libobj/instance.c
  libobj/intersect.c
  libobj/list.c
  libobj/plane.c
  libobj/poly.c
  libobj/roots.c
  libobj/sphere.c
  libobj/torus.c
  libobj/triangle.c
)

set(LIBOBJ_HEADER
  include/libobj/blob.h
  include/libobj/bounds.h
  include/libobj/box.h
  include/libobj/cone.h
  include/libobj/csg.h
  include/libobj/cylinder.h
  include/libobj/disc.h
  include/libobj/geom.h
  include/libobj/grid.h
  include/libobj/hf.h
  include/libobj/instance.h
  include/libobj/list.h
  include/libobj/plane.h
  include/libobj/poly.h
  include/libobj/sphere.h
  include/libobj/torus.h
  include/libobj/triangle.h
)

set(LIBLIGHT_SRC
  liblight/extended.c
  liblight/infinite.c
  liblight/jittered.c
  liblight/light.c
  liblight/point.c
  liblight/shadow.c
  liblight/spot.c
)

set(LIBLIGHT_HEADER
  include/liblight/extended.h
  include/liblight/infinite.h
  include/liblight/jittered.h
  include/liblight/light.h
  include/liblight/point.h
  include/liblight/spot.h
)

set(LIBSURF_SRC
  libsurf/atmosphere.c
  libsurf/fog.c
  libsurf/fogdeck.c
  libsurf/mist.c
  libsurf/surface.c
  libsurf/surfshade.c  
)

set(LIBSURF_HEADER
  include/libsurf/atmosphere.h
  include/libsurf/fogdeck.h
  include/libsurf/fog.h
  include/libsurf/mist.h
  include/libsurf/surface.h
)

set(LIBTEXT_SRC
  libtext/blotch.c
  libtext/bump.c
  libtext/checker.c
  libtext/cloud.c
  libtext/fbmbump.c
  libtext/fbm.c
  libtext/gloss.c
  libtext/imagetext.c
  libtext/mapping.c
  libtext/marble.c
  libtext/mount.c
  libtext/noise.c
  libtext/sky.c
  libtext/stripe.c
  libtext/textaux.c
  libtext/texture.c
  libtext/windy.c
  libtext/wood.c
)

set(LIBTEXT_HEADER
  include/libtext/blotch.h
  include/libtext/bump.h
  include/libtext/checker.h
  include/libtext/cloud.h
  include/libtext/fbmbump.h
  include/libtext/fbm.h
  include/libtext/gloss.h
  include/libtext/imagetext.h
  include/libtext/mapping.h
  include/libtext/marble.h
  include/libtext/mount.h
  include/libtext/sky.h
  include/libtext/stripe.h
  include/libtext/texture.h
  include/libtext/windy.h
  include/libtext/wood.h
)

set(LIBRARY_SRC
  ${LIBCOMMON_SRC}
  ${LIBIMAGE_SRC}
  ${LIBOBJ_SRC}
  ${LIBLIGHT_SRC}
  ${LIBSURF_SRC}
  ${LIBTEXT_SRC}
)

set(LIBRARY_HEADER
  ${LIBCOMMON_HEADER}
  ${LIBIMAGE_HEADER}
  ${LIBOBJ_HEADER}
  ${LIBLIGHT_HEADER}
  ${LIBSURF_HEADER}
  ${LIBTEXT_HEADER}
)

add_library(ray_static ${LIBRARY_SRC})
set_target_properties(ray_static PROPERTIES SOVERSION 1)
target_link_libraries(ray_static)

add_library(ray SHARED ${LIBRARY_SRC})
set_target_properties(ray PROPERTIES SOVERSION 1)
target_link_libraries(ray)
