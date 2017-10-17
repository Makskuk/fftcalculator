TEMPLATE = subdirs

# Ensure that library is built before application
CONFIG  += ordered

SUBDIRS += \
    fftreal \
    app
