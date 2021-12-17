#!/usr/bin/env python3

from scipy.io.wavfile import read
from scipy.fftpack import fft,fftfreq
from scipy.io import wavfile # get the api
import matplotlib.pyplot as plt
import numpy as np
import wave
import sys

# Figure 1
fs, data = wavfile.read('./../datasets/169A-Y582523-202110120957ALU.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
# c = fft(a) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([None, 1500])
# plt.title('169A-Y582523-202110120957ALU.WAV');
xf = fftfreq(len(data), 1/fs)[:len(data)//2]
plt.plot(xf, abs(c[:(d-1)]),'b') 

# plt.show()
# plt.figure();


# plt.figure();
fs, data = wavfile.read('./../datasets/167A-Y582523-202110120956-RU.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
# c = fft(a) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([None, 1500])
plt.title('[169A-Y582523-202110120957ALU.WAV(Blue)][167A-Y582523-202110120956-RU.WAV(Red)]');
plt.plot(abs(c[:(d-1)]),'r') 
# plt.show()

# Figure 1

plt.figure()

fs, data = wavfile.read('./../datasets/173A-Y333222-202110121009ALU.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
# c = fft(a) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([5200, 5300])
ax.set_ylim([None, 1500])
# plt.title('169A-Y582523-202110120957ALU.WAV');
plt.plot(abs(c[:(d-1)]),'b') 
# plt.show()
# plt.figure();

# plt.figure()
fs, data = wavfile.read('./../datasets/175A-Y333222-202110121009ARU.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
# c = fft(a) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes 
ax.set_xlim([5200, 5300])
ax.set_ylim([None, 1500])
plt.title('[173A-Y333222-202110121009ALU.WAV(Blue)][175A-Y333222-202110121009ARU.WAV.WAV(Red)]');
plt.plot(abs(c[:(d-1)]),'r') 
# plt.show()
plt.figure();

fs, data = wavfile.read('./../datasets/078A-202110292051共振4桌子.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([0, 7000])
# plt.title('169A-Y582523-202110120957ALU.WAV');
plt.plot(abs(c[:(d-1)]),'b') 
# plt.show()
# plt.figure();

fs, data = wavfile.read('./../datasets/079A-202110292051.共振4桌子.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([0, 7000])
plt.plot(abs(c[:(d-1)]),'r') 

fs, data = wavfile.read('./../datasets/080A-202110292052共振4桌子..WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([0, 7000])
plt.title('[078A-202110292051共振4桌子(Blue)][079A-202110292051.共振4桌子(Red)][080A-202110292052共振4桌子(Yellow)]', fontproperties="AR PL UMing CN");
plt.plot(abs(c[:(d-1)]),'y') 
# plt.show()
plt.figure();

fs, data = wavfile.read('./../datasets/096A-202110292059共振4正常人.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
# ax.set_ylim([None, 80000000])
# plt.title('169A-Y582523-202110120957ALU.WAV');
plt.plot(abs(c[:(d-1)]),'b') 
# plt.show()
# plt.figure();

fs, data = wavfile.read('./../datasets/097A-202110292059共振4正常人.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
# ax.set_ylim([None, 80000000])
plt.plot(abs(c[:(d-1)]),'r') 


fs, data = wavfile.read('./../datasets/098A-202110292059共振4正常人.WAV') # load the data
# a = data.T[0] # this is a two channel soundtrack, I get the first track
a = data  # this is a 1 channel soundtrack
b=[(ele/2**16.)*2-1 for ele in a] # this is 16-bit track, b is now normalized on [-1,1)
c = fft(b) # calculate fourier transform (complex numbers list)
d = int(len(c)/2)  # you only need half of the fft list (real signal symmetry)
ax = plt.gca() # get current axes
ax.set_xlim([2600, 2625])
ax.set_ylim([0, 5000])
plt.title('[096A-202110292059共振4正常人(Blue)][097A-202110292059共振4正常人(Red)][098A-202110292059共振4正常人(Yellow)]', fontproperties="AR PL UMing CN");
plt.plot(abs(c[:(d-1)]),'y') 
# plt.show()
plt.figure();


# read audio samples
# input_data = read("./../datasets/169A-Y582523-202110120957ALU.WAV")
# audio = input_data[1]
# # plot the first 1024 samples
# # plt.plot(audio[0:1024])
# plt.plot(audio[0:])
# # label the axes
# plt.ylabel("Amplitude")
# plt.xlabel("Time")
# # set the title  
# plt.title("169A-Y582523-202110120957ALU.WAV")
# display the plot
plt.show()



# if __name__ == "__main__":
#     with open("./../../datasets/169A-Y582523-202110120957ALU.WAV", 'rb') as file:
#         byte = file.read(1)
#         while byte:
#             print(hex(byte[0])) # how to print hex instead of ascii?
#             byte = file.read(1)