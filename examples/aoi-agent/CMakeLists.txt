# CMake minimum version
cmake_minimum_required(VERSION 3.10)
build_example(
  NAME aoi
  SOURCE_FILES 
    main.cc 
    ap-app.cc
    server-app.cc 
    user-app.cc 
    status-util.cc
    logger.cc
    fwd-service.cc
    server-queue.cc
#    test-tag.cc
    ap-app.h
    server-app.h
    user-app.h
    status-util.h
    logger.h
    json.hpp
    fwd-service.h
    event-time-tag.h
    packet-type-tag.h
    server-queue.h
#    result-classifier.h
#    test-tag.h


  LIBRARIES_TO_LINK
    ${ns3-libs}
    ${ns3-contrib-libs}
    # nlohmann_json::nlohmann_json
    # SQLite::SQLite3
    # ${libcsma}
    # ${libinternet}
    # ${libapplications}
)