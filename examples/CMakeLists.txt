build_lib_example(
  NAME interference-pattern
  SOURCE_FILES interference-pattern/mygym.cc
               interference-pattern/sim.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME linear-mesh
  SOURCE_FILES linear-mesh/sim.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME linear-mesh-2
  SOURCE_FILES linear-mesh-2/sim.cc
               linear-mesh-2/mygym.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME opengym
  SOURCE_FILES opengym/sim.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libopengym}
)

build_lib_example(
  NAME opengym-2
  SOURCE_FILES opengym-2/sim.cc
               opengym-2/mygym.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libopengym}
)

build_lib_example(
  NAME aoi-agent
  SOURCE_FILES aoi-agent/sim.cc
#        aoi-agent/main.cc
        aoi-agent/ap-app.cc
        aoi-agent/server-app.cc
        aoi-agent/user-app.cc
        aoi-agent/status-util.cc
        aoi-agent/logger.cc
        aoi-agent/fwd-service.cc
        aoi-agent/server-queue.cc
        aoi-agent/ap-app.h
        aoi-agent/server-app.h
        aoi-agent/user-app.h
        aoi-agent/status-util.h
        aoi-agent/logger.h
        aoi-agent/json.hpp
        aoi-agent/fwd-service.h
        aoi-agent/event-time-tag.h
        aoi-agent/packet-type-tag.h
        aoi-agent/server-queue.h
  LIBRARIES_TO_LINK
    ${libcore}
    ${ns3-libs}
    ${ns3-contrib-libs}
    ${libopengym}
)

build_lib_example(
  NAME rl-tcp
  SOURCE_FILES rl-tcp/sim.cc
               rl-tcp/tcp-rl-env.cc
               rl-tcp/tcp-rl.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libflow-monitor}
    ${libinternet}
    ${libopengym}
    ${libpoint-to-point-layout}
    ${libptp}
)
