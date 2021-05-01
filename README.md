# TCP-File-Downloader
C code for a TCP client that downloads a file using an HTTPS or HTTP based URL and saves it locally.  
If the URL is wrong, it displays a message accordingly.

## Step1: Download OpenSSL library
OpenSSL can be downloaded using `sudo apt-get install libssl-dev` on the command line.

## Step2: Compilation of the code
`make`

## Step3: Running the code
The code can be executed using `./client <URL>`  
For instance,  
1. An example of an HTTP file: `./client http://cdn4.vectorstock.com/i/1000x1000/65/18/letter-g-painted-brush-vector-20406518.jpg`  
2. An example of an HTTPS file: `./client https://www.cdc.gov/healthypets/images/pets/cute-dog-headshot.jpg`  
For the HTTPS file, it would be saved in the folder as `"cute-dog-headshot.jpg"`. If it is called again, it will be overwritten.

## Step4: Enjoy the bonus dog pics!
:)
