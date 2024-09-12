function animation(matrix, delay)



plot_matrix_animation(real(matrix));



function plot_matrix_animation(matrix)
    % Get the dimensions of the matrix
    [num_rows, num_cols] = size(matrix);

    % Create a figure and axis
    figure;
    ax = gca;

    % Set the axis limits based on the matrix values
    xlim([1, num_cols]);
    ylim([min(matrix(:)), max(matrix(:))]);

    % Create a plot handle for the line
    plot_handle = plot(1:num_cols, matrix(1, :), 'b-');

    % Set up the animation loop
    for row_idx = 1:num_rows
        % Update the y-data of the plot
        set(plot_handle, 'ydata', matrix(row_idx, :));

        % Draw the plot
        drawnow;

        % Pause for a short time
        pause(delay);
    end
end


end