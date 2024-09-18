add_definitions(-DQCUSTOMPLOT_COMPILE_LIBRARY)
#
add_library(qcustomplot
        qcustomplot.cpp
        qcustomplot.h
)

target_link_libraries(qcustomplot PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

target_link_libraries(qcustomplot PRIVATE
        Qt6::PrintSupport
)