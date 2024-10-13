# SmartLog Installation Guide

## Installation (Windows)

1. Extract the ZIP file to `C:\win`.
2. Modify system environment variables:
   - Go to **System Properties** >> **Advanced** >> **Environment Variables**.
   - Under **User Variables** for `<User>`, find and select **Path**, then click **Edit**.
   - Click **New**, then add: `C:\win\SmartLog`.
   - Confirm with **OK** on all windows.
3. Open **Command Prompt** and type `SmartLog -help` to get usage instructions.

## Installation (Linux)

1. Extract the ZIP file to `/opt`.
2. Edit your `.bashrc` file:
   ```bash
   nano ~/.bashrc
3. Add following line to the end of the file:
    ```bash 
    export PATH=$PATH:/opt/SmartLog
4. Save the file by pressing Ctrl + X, then Y and Enter.
5. Apply the changes by running:
    ```bash
    source ~/.bashrc
6. Open a terminal and type `SmartLog -help` to get usage instructions.
