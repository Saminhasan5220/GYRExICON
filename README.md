# Log data from an ESP8266 to Google Sheets

###### Below are step by step instructions to begin logging data using an ESP8266 module without the need for a third party service. This will publish random number values, so no additional hardware or sensors are required for the script to run. The HTTPSRedirect library is used to publish the data. More information on that library can be found here: https://github.com/Saminhasan5220/HTTPSRedirect

<br>

## Instructions for Google Sheets

1. Create new Google Sheets document, and give it a name.

2. Type the following text into each of the following cells:

   - Cell A1: `Date`
   - Cell B1: `Time`
   - Cell C1: `value0`
   - Cell D1: `value1`
   - Cell E1: `value2`
   <br>


   
3. Get the Spreadsheet ID from the URL.

   

4. From the Google Sheets menu, go to `Tools > Script Editor`
   
   *Note:  Make sure you are using the new Apps Script editor (not the legacy version). The editor will default to the new version, but if you're using the legacy version you'll need switch back by clicking the blue "Use new editor" button at the top of the page.*

5. Delete all of the default text in the script editor, and paste the GoogleScripts-example.gs code.

6. Update the Spreadsheet ID (line 12) with the ID obtained in step 3, and click `Save`.

   *Note:  The Spreadsheet ID must be contained in single quotation marks as shown in the example code, and **the script must be saved before continuing** to the next step.*
   
7. Click the blue `Deploy` button at the top right of the page, and select `New Deployment`. 
 
   Click the `gear` icon next to Select Type, and select  `Web App` and modify the following:

   - Enter a Description (optional)
   - Execute as: `Me`
   - Who has access: `Anyone` *(note: do not select `Anyone with a Google Account` - you must scroll down to the bottom to find `Anyone`)*
   
   Click `Deploy` 
   
8. Click `Authorize access` then select your Google account.
   
   On the "Google hasn't verified this app" screen, select `Advanced` > `Go to Untitled project (unsafe)` > `Allow`

   Copy and save the `Deployment ID` for use in the ESP8266 code, and click `Done`.

9. From the script editor, click `Save` and then `Run`. 

   *Note:  Nothing will happen when you click run but you must do that once before continuing.*
 


   

## Instructions for ESP8266

1. In the Arduino IDE, paste the "ESP8266-example.cpp" code into a blank sketch. Overwrite any existing code that was there.

2. Update the following info:

    - Add your Wifi network name
    - Add your Wifi password
    - Replace the Google Script Deployment ID with the ID obtained in step 8 above
    
    *Note:  The Deployment ID must be contained in quotation marks as shown in the example code. The Deployment ID can also be found by clicking `Deploy > Manage Deployments`.*

3. Install the HTTPSRedirect library from here:

    https://github.com/electronicsguy/HTTPSRedirect

    (click on the green "code" button and "Download ZIP", then unzip the file and move the HTTPSRedirect folder to your library directory)

4. Upload code to your ESP8266 module and watch data get published to your sheet!

     

## Troubleshooting and Additional Notes

1. If you get an "Error compiling for board..." related to the HTTPSRedirect library (and the GScriptID, host, password, and ssid definitions), go the the HTTPSRedirect library folder and delete the "config.cpp" file. Note that the network ID, password, host, and Google Script Deployment ID are defined in the main ESP8266 example code and therefore this file is not required and may cause errors for some compilers. 
2. When making changes to the Google Scripts code, you will need to click `Save` then `Deploy > New deployment` for any new changes to take effect. You will be given a new Deployment ID that you will have to update in the ESP8266 code each time (each new deployment is given a new Deployment ID).
3. For applications where data is not sent on a regular interval but rather on an event trigger (such as only sending data when a button is pressed), occasionally the first attempt to send data will fail. Therefore, the code must be modified to send the data again if the first attempt fails. See the example under the "Advanced" folder for additional information.
4. This tutorial does not work for an ESP32 device.

