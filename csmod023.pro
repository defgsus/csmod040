#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T12:29:35
#
#-------------------------------------------------

TARGET = csmod023
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lfftw3f -lportaudio -lpng -lfltk

HEADERS += \
    csmodcontainer.h \
    csmod.h \
    csmod_installed_modules.h \
    csmod_menu.h \
    csmod_midi.h \
    csmodule_analyze.h \
    csmodule_audiofile.h \
    csmodule_audioin.h \
    csmodule_audioout.h \
    csmodule_band.h \
    csmodule_beta.h \
    csmodule_bit.h \
    csmodule_ca.h \
    csmodule_camera.h \
    csmodule_clip.h \
    csmodule_clockdiv.h \
    csmodule_comp.h \
    csmodule_const.h \
    csmodule_container.h \
    csmodule_control.h \
    csmodule_delay.h \
    csmodule_display.h \
    csmodule_env.h \
    csmodule_fft.h \
    csmodule_filter.h \
    csmodule.h \
    csmodule_logic.h \
    csmodule_math.h \
    csmodule_midi.h \
    csmodule_mixer.h \
    csmodule_nn.h \
    csmodule_objects.h \
    csmodule_osc.h \
    csmodule_p2f.h \
    csmodule_panning.h \
    csmodule_rnd.h \
    csmodule_rotate.h \
    csmodule_sampler.h \
    csmodule_scope2d.h \
    csmodule_scope3d.h \
    csmodule_select.h \
    csmodule_seq.h \
    csmodule_sh.h \
    csmodule_sum.h \
    csmodule_synth.h \
    csmodule_system.h \
    csmodule_table.h \
    csmod_wavereader.h \
    csmod_wavewriter.h \
    system.h \
    csmodule_scopegl.h

SOURCES += \
    main.cpp \
    csmodcontainer.cpp \
    csmod.cpp \
    csmod_installed_modules.cpp \
    csmod_menu.cpp \
    csmod_midi.cpp \
    csmodule_analyze.cpp \
    csmodule_audiofile.cpp \
    csmodule_audioin.cpp \
    csmodule_audioout.cpp \
    csmodule_band.cpp \
    csmodule_beta.cpp \
    csmodule_bit.cpp \
    csmodule_ca.cpp \
    csmodule_camera.cpp \
    csmodule_clip.cpp \
    csmodule_clockdiv.cpp \
    csmodule_comp.cpp \
    csmodule_const.cpp \
    csmodule_container.cpp \
    csmodule_control.cpp \
    csmodule.cpp \
    csmodule_delay.cpp \
    csmodule_display.cpp \
    csmodule_env.cpp \
    csmodule_fft.cpp \
    csmodule_filter.cpp \
    csmodule_logic.cpp \
    csmodule_math.cpp \
    csmodule_midi.cpp \
    csmodule_mixer.cpp \
    csmodule_nn.cpp \
    csmodule_objects.cpp \
    csmodule_osc.cpp \
    csmodule_p2f.cpp \
    csmodule_panning.cpp \
    csmodule_rnd.cpp \
    csmodule_rotate.cpp \
    csmodule_sampler.cpp \
    csmodule_scope2d.cpp \
    csmodule_scope3d.cpp \
    csmodule_scopegl.cpp \
    csmodule_select.cpp \
    csmodule_seq.cpp \
    csmodule_sh.cpp \
    csmodule_sum.cpp \
    csmodule_synth.cpp \
    csmodule_system.cpp \
    csmodule_table.cpp \
    csmod_wavereader.cpp \
    csmod_wavewriter.cpp \
    system.cpp

