import numpy as np
from scipy.signal import stft, butter, filtfilt
from scipy.ndimage import median_filter
import csv
import os

# waterfall
def spectrogram_data(complex_data, fs):
    # Compute the Short-Time Fourier Transform (STFT)
    nperseg = 256*8  # Number of samples per segment
    noverlap = nperseg // 8  # Number of overlapping samples
    frequencies, times, Zxx = stft(complex_data, fs=fs, nperseg=nperseg, noverlap=noverlap)


    # Shift the zero frequency component to the center
    Zxx_shifted = np.fft.fftshift(Zxx, axes=0)
    frequencies_shifted = np.fft.fftshift(frequencies)

    # Compute the magnitude of the STFT
    magnitude_spectrogram = np.abs(Zxx_shifted)
    threshold_dB = 40
    threshold = 10**(threshold_dB / 20)  # Convert dB to linear scale

    magnitude_spectrogram_db = 20 * np.log10(magnitude_spectrogram)   
    return times, frequencies_shifted, magnitude_spectrogram_db


def notch_filter(complex_data, fs, q, cutoff, order):
    # Notch filter
    # Parameters
    notch_freq = 0  # Notch filter centered at 0 Hz (DC)
    quality_factor = q  # Q-factor for the notch filter
    cutoff_freq = cutoff  # 
    nyquist_rate = fs / 2
    normalized_cutoff = cutoff_freq / nyquist_rate
    b, a = butter(order, normalized_cutoff, btype='high', analog=False)
    complex_data = filtfilt(b, a, complex_data)
    return complex_data

def median_filter_complex(complex_data, size):
    real_data = median_filter(np.real(complex_data),size)
    imag_data = median_filter(np.imag(complex_data), size)
    return real_data + 1j*imag_data

def read_iq_bin(file_path):
    # Read the binary file
    with open(file_path, 'rb') as file:
        # Read the data into a numpy array
        # The dtype should match the type of data in the file (double = float64)
        data = np.fromfile(file, dtype=np.float64)

    # Reshape or process the data if needed
    # Here we assume data was written as 2 doubles per sample
    num_new_samples = int(len(data)/2)
    data = data.reshape((num_new_samples, 2))

    # Create complex numbers from the real and imaginary parts
    complex_data = data[:, 0] + 1j * data[:, 1]
    return complex_data

def read_complex_csv(file_path):
    complex_data = []
    
    # Open the CSV file for reading
    with open(file_path, 'r', newline='') as csvfile:
        csvreader = csv.reader(csvfile)
        
        # Read each row and reconstruct the complex number
        for row in csvreader:
            if len(row) == 2:
                real_part = float(row[0])
                imag_part = float(row[1])
                complex_number = real_part + 1j * imag_part
                complex_data.append(complex_number)
    
    # Convert the list of complex numbers into a numpy array
    return np.array(complex_data)

def extract_signals_from_bin(file_path, N):
    # Check if file exists
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"File {file_path} does not exist.")

    # Read the file in binary mode
    with open(file_path, 'rb') as f:
        data = np.fromfile(f, dtype=np.float64)


    # Split into chunks of size N
    offset = len(data) % N
    if offset != 0:
        data = data[0:-offset]
    

    num_chunks = len(data) // N
    data = data.reshape((num_chunks, N))

    return data

def gen_chirp_to_csv(file_path, start, stop, length, separation, amplitude, fs):
    result = []

    for i in range(int(length)):
        t = i / fs
        frequency = start + (stop - start) * i / length
        phase = 2.0 * np.pi * frequency * t
        sample = amplitude * np.exp(1j * phase) + 0j 
        result.append(sample)

    # appending zeros so transmitting is "turned off"
    num_zeros = int(separation)
    for i in range(num_zeros):
        result.append(0)
    result = np.array(result)

    # Write to CSV file
    with open(file_path, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        for sample in result:
            csvwriter.writerow([sample.real, sample.imag])
    return result

def gen_chirp_to_csv_rep(file_path, start, stop, width, separation, amplitude, fs, num_pulses):
    result = []

    for i in range(width):
        t = i / fs
        frequency = start + (stop - start) * i / width
        phase = 2.0 * np.pi * frequency * t
        sample = amplitude * np.exp(1j * phase) + 0j 
        result.append(sample)

    # appending zeros so transmitting is "turned off"
    num_zeros = int(separation)
    for i in range(num_zeros):
        result.append(0)
    result = np.array(result)
    result = np.tile(result, num_pulses)

    # Write to CSV file
    with open(file_path, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        for sample in result:
            csvwriter.writerow([sample.real, sample.imag])
    return result
