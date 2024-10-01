import numpy as np
import matplotlib.pyplot as plt
import time
import os
import csv
from scipy.signal import fftconvolve

# CONSTS
c = 3e8

# PARAMS
fs = 25e6
pulse_width = 5000
pulse_sep = 5000

# File path to the data file
file_path_bin = 'outputs/pulsed_test.bin'
file_path_csv = 'sweep.csv'
file_path_res = 'outputs/signal.bin'
if os.path.exists(file_path_res):
    os.remove(file_path_res)
    print(f"{file_path_res} has been deleted.")


def append_signal_to_bin(file_path, signal):
    if (len(signal)<500):
        print(f"Length of signal {len(signal)} is less than 500")
        return
    # Ensure the signal is a numpy array
    signal = np.array(signal)

    # Open the file in append mode and write the data
    with open(file_path, 'ab') as f:  # 'ab' mode to append as binary
        signal.tofile(f)

def extract_signals_from_bin(file_path, N):
    # Check if file exists
    if not os.path.exists(file_path):
        print(f"{file_path} does not exist.")
        return []


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

def read_iq_bin(file_path):
    try:
        # Read the binary file
        with open(file_path, 'rb') as file:
            # Read the data into a numpy array
            data = np.fromfile(file, dtype=np.float64)
            
        # Check if the file is empty
        if len(data) == 0:
            print("File is empty.")
            return None
        
        # Reshape or process the data if needed
        num_new_samples = int(len(data) / 2)
        if num_new_samples == 0:
            return None
        
        data = data.reshape((num_new_samples, 2))

        # Create complex numbers from the real and imaginary parts
        complex_data = data[:, 0] + 1j * data[:, 1]
        return complex_data

    except Exception as e:
        print(f"Error reading file: {e}")
        return None
    
def update_plot_data(data):
        # PLOT FFT
        fft_data = 20*np.log10(np.fft.fft(data))
        fft_freq_axis = np.fft.fftfreq(len(fft_data),d=1/fs)
        line_fft.set_xdata(fft_freq_axis)  # Set x-axis data (assume index is x)
        line_fft.set_ydata(fft_data)  # Update y-axis with the latest data
        axs[0,0].relim()  # Recompute limits
        axs[0,0].autoscale_view(True, True, True)  # Rescale the plot based on new data

        # PLOT TIME
        line_time.set_xdata(np.arange(len(data)))  # Set x-axis data (assume index is x)
        line_time.set_ydata(data)  # Update y-axis with the latest data
        axs[1,0].relim()  # Recompute limits
        axs[1,0].autoscale_view(True, True, True)  # Rescale the plot based on new data

        # PLOT XCORR
        xcorr_data = fftconvolve(data, matched_filter, mode='full')
        xcorr_data = xcorr_data[80000:-5000] # Artifact at start
        line_xcorr.set_xdata(np.arange(len(xcorr_data)))  # Set x-axis data (assume index is x)
        line_xcorr.set_ydata(np.abs(xcorr_data))  # Update y-axis with the latest data
        axs[2,0].relim()  # Recompute limits
        axs[2,0].autoscale_view(True, True, True)  # Rescale the plot based on new data

        # PLOT EXTRACTED POWER
        max_idx = np.argmax(np.abs(xcorr_data))
        xcorr_extracted = np.abs(xcorr_data[max_idx:max_idx+pulse_width+pulse_sep])
        #xcorr_extracted = np.abs(xcorr_extracted)**2
        time_delays = np.arange(len(xcorr_extracted)) / fs  # Time delay for each sample
        ranges = 0.8 * c * time_delays / 2  # Range in meters
        end = 500
        ranges = ranges[0:end] # trying to see around 100/200m
        xcorr_extracted = xcorr_extracted[0:end]
        append_signal_to_bin(file_path_res,xcorr_extracted)
        line_xcorr_argmax.set_xdata(ranges)
        #line_xcorr_argmax.set_xdata(np.arange(len(xcorr_extracted)))
        line_xcorr_argmax.set_ydata(20*np.log10(xcorr_extracted))
        axs[3,0].relim()  # Recompute limits
        axs[3,0].autoscale_view(True, True, True)  # Rescale the plot based on new data


        # PLOT EXTRACTED FMCW PULSE
        pulse_extracted = data[max_idx-pulse_width:max_idx]
        line_pulse.set_xdata(np.arange(len(pulse_extracted)))
        line_pulse.set_ydata(pulse_extracted)
        axs[0,1].relim()  # Recompute limits
        axs[0,1].autoscale_view(True, True, True)  # Rescale the plot based on new data

        # PLOT TEMPLATE PULSE
        #line_template.set_xdata(np.arange(len(template_signal)))
        #line_template.set_ydata(template_signal)
        #axs[1,1].relim()  # Recompute limits
        #axs[1,1].autoscale_view(True, True, True)  # Rescale the plot based on new data

        # PLOT AVERAGE SIGNAL AND DIFF
        signal_mat = extract_signals_from_bin(file_path_res, end)
        if (len(signal_mat) > 0 and len(ranges) == end):
            line_11.set_xdata(ranges)
            line_11.set_ydata(20*np.log10(np.abs(ave_crosstalk)))
            axs[1,1].relim()  # Recompute limits
            axs[1,1].autoscale_view(True, True, True)  # Rescale the plot based on new data

            ave_signal = np.mean(signal_mat, axis=0)
            ave_signal = ave_signal[0:end]
            #line_21.set_xdata(np.arange(len(ave_crosstalk)))
            line_21.set_xdata(ranges)
            line_21.set_ydata(20*np.log10(np.abs(ave_signal)))
            axs[2,1].relim()  # Recompute limits
            axs[2,1].autoscale_view(True, True, True)  # Rescale the plot based on new data

            diff = ave_signal - ave_crosstalk
            line_31.set_xdata(ranges)
            line_31.set_ydata(20*np.log10(np.abs(diff)))
            axs[3,1].relim()  # Recompute limits
            axs[3,1].autoscale_view(True, True, True)  # Rescale the plot based on new data



        '''
        # PLOT FFT
        fft_data = 20*np.log10(np.fft.fft(pulse_extracted))
        fft_freq_axis = np.fft.fftfreq(len(fft_data),d=1/fs)
        line_fft_pulse.set_xdata(fft_freq_axis)  # Set x-axis data (assume index is x)
        line_fft_pulse.set_ydata(fft_data)  # Update y-axis with the latest data
        axs[2,1].relim()  # Recompute limits
        axs[2,1].autoscale_view(True, True, True)  # Rescale the plot based on new data


        # PLOT MIXED FMCW PULSE
        #template_signal = template_signal[0:pulse_width]
        pulse_mixed = data[max_idx:max_idx+pulse_width] * np.conj(template_signal[0:pulse_width])
        N = len(pulse_mixed)
        fft_result = 20*np.log10(np.abs(np.fft.fft(pulse_mixed)))
        frequencies = np.fft.fftfreq(N, 1/fs)
        cutoff = 0
        frequencies = frequencies[:N//2-cutoff]
        fft_result = fft_result[:N//2-cutoff]
        beat_freq = frequencies[np.argmax(fft_result)]
        print(beat_freq)
        #ranges = c / (2 * frequencies)  # R = c / (2f)
        #ranges[~np.isfinite(ranges)] = 0 
        line_mixed.set_xdata(frequencies)
        line_mixed.set_ydata(fft_result)
        axs[3,1].relim()  # Recompute limits
        axs[3,1].autoscale_view(True, True, True)  # Rescale the plot based on new data
        '''
        

# Initialize plot
plt.ion()  # Enable interactive mode
fig, axs = plt.subplots(4, 2, figsize=(20, 12))

line_fft, = axs[0,0].plot([], [], 'b-')  # Empty plot
line_time, = axs[1,0].plot([], [], 'b-')  # Empty plot
line_xcorr, = axs[2,0].plot([], [], 'b-') 
line_xcorr_argmax, = axs[3,0].plot([], [], 'b-') 
line_pulse, = axs[0,1].plot([], [], 'b-') 
line_11, = axs[1,1].plot([], [], 'b-') 
line_21, = axs[2,1].plot([], [], 'b-') 
line_31, = axs[3,1].plot([], [], 'b-') 


# Store previous data to retain it if file read fails
previous_data = None

def update_plot(data):
    global previous_data
    
    if data is not None:
        print("Updating plot with new data.")
        previous_data = data  # Update previous data only if new data is valid
        update_plot_data(data)


        fig.canvas.draw()  # Redraw the plot
        fig.canvas.flush_events()  # Ensure the update is reflected on the plot
    else:
        # Retain the previous plot if no new valid data
        print("Retaining previous plot.")

        update_plot_data(previous_data)

        fig.canvas.draw()
        fig.canvas.flush_events()

# Main loop to continuously check for file updates
last_modified_time = 0
template_signal = read_complex_csv(file_path_csv)
template_signal = template_signal[0:pulse_width]
matched_filter = np.conjugate(template_signal[::-1])

end = 500
crosstalk_mat = extract_signals_from_bin("outputs/crosstalk.bin", end)
ave_crosstalk = np.mean(crosstalk_mat, axis=0)
ave_crosstalk = ave_crosstalk[0:end]

while True:
    try:
        # Check if the file has been modified
        current_modified_time = os.path.getmtime(file_path_bin)
        current_file_size = os.path.getsize(file_path_bin)
        
        # If the file size is stable and it has been modified, update the plot
        if current_modified_time != last_modified_time and current_file_size > 0:
            time.sleep(0.1)  # Short delay to ensure the file isn't being written
            last_modified_time = current_modified_time
            data = read_iq_bin(file_path_bin)
            update_plot(data)
        
        time.sleep(1)  # Poll the file every 1 second

    except KeyboardInterrupt:
        print("Plotting stopped by user.")
        break
