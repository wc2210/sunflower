set(base_SRCS
  Hash.cc
  ThreadPool.cc
  TaskThreadPool.cc
  )

add_library(sunflower_base ${base_SRCS})
target_link_libraries(sunflower_base pthread)
