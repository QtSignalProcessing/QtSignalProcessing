Documentation of the real world signal projects

Libraries:
         fftw3 for FFT,
         libsndfile for reading in the audio data,
         phonon for playing the audio file,
         OpenGL for plotting

7 classes: //TOP-DOWN approach
        
TOP LEVEL:
class mainwindow : create the mainwindow with MenuBar, read in the default audio file, get the information of the file(data,sampling rate,time,number of frames), data, time and number of items are used to create PlotSW(Plot Sampled Waveform) widget and Plot(the original signal) widget, a bool flag "discrete" is used to indicate the plotOSpectrum is ploting the continuous-time spectrum or the discrete-time spectrum, a spinbox is in charge of the quantization(signal is connected to the slots glospectrum::setBits to compute the FFT of the quantized data and slots glsw::setBits to get the quantized data)
                
           
         slots : play()=play the original audio file,triggered by clicking the upper "play" button;
                 open()=let the user to select an audio file via GUI,triggered by clicking the File->open;
                 about()=display the information about this project, perhaps later on display this file?,triggered
                   by clicking the Help->about;
         
