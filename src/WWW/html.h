static const char indexData[] PROGMEM = R"==(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>K2 RFID</title>
  <style>
    .btn {
      background-color: #0047AB;
      border: none;
      color: #FFFFFF;
      padding: 16px 30px 16px 30px;
      font-size: 16px;
      cursor: pointer;
      font-weight: bold;
      margin: 10px;
      border-radius: 30px;
    }

    .btn:hover {
      background-color: #1976D2;
    }

    svg {
      fill: #0047AB;
      background-color: #E0E0E0;
      border: none;
      cursor: pointer;
    }

    svg:hover {
      fill: #1976D2;
      background-color: #E0E0E0;
    }

    body {
      font-family: arial, sans-serif;
      background-color: #E0E0E0;
      margin: 0;
    }

    label {
      color: #000000;
      font-size: 18px;
      font-weight: bold;
      font-family: arial, sans-serif;
    }

    .main {
      padding: 0;
      position: absolute;
      top: 0;
      right: 0;
      bottom: 0;
      left: 0;
      background-color: #E0E0E0;
      font-size: 16px;
      font-weight: bold;
      color: #000000;
      font-family: arial, sans-serif;
    }

    input[type="color"] {
      border: 1px solid #000000;
      background-color: #ffffff;
      width: 140px;
      height: 40px;
      margin: 10px;
      cursor: pointer;
    }

    table {
      font-family: arial, sans-serif;
      border-collapse: collapse;
    }

    select {
      height: 40px;
      width: 140px;
      padding: 10px;
      font-size: 16px;
      margin: 10px;
      cursor: pointer;
    }

    .msg {
      color: #000000;
      font-size: 16px;
      font-weight: bold;
      text-shadow: none;
    }

    .errormsg {
      color: #ff0000;
      font-size: 16px;
      font-weight: bold;
      text-shadow: none;
    }

    dialog {
      padding: 0 !important;
      border: none;
      margin: 0;
      min-width: 100%;
      min-height: 100%;
      max-width: 100%;
      max-height: 100%;
      background-color: #E0E0E0;
    }

    td {
      border: none;
      text-align: left;
      padding: 8px;
    }

    th {
      border: none;
      color: white;
      background-color: #154c79;
      text-align: center;
      padding: 8px;
    }

    input[type="text"] {
      padding: 10px;
      font-size: 16px;
      margin: 2px;
      border: 1px solid #000000;
      cursor: pointer;
    }

    input[type="text"]:invalid {
      border: 1px solid red;
    }

    input[type="text"]:invalid:focus {
      background-color: rgba(255, 0, 0, 0.486);
      background-blend-mode: overlay;
    }

    progress {
      border-radius: 6px;
      width: 200px;
      height: 16px;
      overflow: hidden;
    }

    progress::-webkit-progress-bar {
      background-color: #ffffff;
      border-radius: 6px;
    }

    progress::-webkit-progress-value {
      background-color: #3498db;
      border-bottom-left-radius: 6px;
      border-top-left-radius: 6px;
      float: left;
    }

    #loader {
      z-index: 1;
      width: 50px;
      height: 50px;
      margin: 0 0 0 0;
      border: 6px solid #f3f3f3;
      border-radius: 50%;
      border-top: 6px solid #3498db;
      width: 50px;
      height: 50px;
      -webkit-animation: spin 2s linear infinite;
      animation: spin 2s linear infinite;
    }

    @-webkit-keyframes spin { 0% { -webkit-transform: rotate(0deg); } 100% { -webkit-transform: rotate(360deg); } }
    @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }

    /* CSS for Toggle Switch */
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; vertical-align: middle; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; }
    .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; }
    input:checked + .slider { background-color: #2196F3; }
    input:focus + .slider { box-shadow: 0 0 1px #2196F3; }
    input:checked + .slider:before { transform: translateX(26px); }
    .slider.round { border-radius: 34px; }
    .slider.round:before { border-radius: 50%; }
    #modeLabel { vertical-align: middle; margin-left: 8px; }
  </style>
  <script>
    var filaments;
    var dbVersion;
    var jsonDb;
    var paramStr;

    // --- Start of Read/Write Mode Variables ---
    var currentSpoolerMode = "write"; // Default to write mode. 'read' or 'write'.
    var modeToggle;
    var modeLabel;
    var loadCardDataBtn;
    // --- End of Read/Write Mode Variables ---

    const compress = byteArray => {
      const cs = new CompressionStream('gzip');
      const writer = cs.writable.getWriter();
      writer.write(byteArray);
      writer.close();
      return new Response(cs.readable).arrayBuffer();
    };

    function sendData() {
      // ... (sendData function remains unchanged) ...
      document.getElementById("message").innerHTML = "<label class=\"msg\">Saving spool settings...</label>";
      var materialBrand = document.getElementById("materialBrand").value;
      var materialType = document.getElementById("materialType").value;
      var materialWeight = document.getElementById("materialWeight").value;
      var materialColor = document.getElementById("materialColor").value;
      var postdata = "materialType=" + materialType + "&materialWeight=" + materialWeight + "&materialColor=" + materialColor;
      localStorage.setItem("materialBrand", materialBrand);
      localStorage.setItem("materialType", materialType);
      localStorage.setItem("materialWeight", materialWeight);
      localStorage.setItem("materialColor", materialColor);
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function () {
        if (this.readyState == 4) {
          if (this.status == 200) {
            document.getElementById("message").innerHTML = "<label class=\"msg\">Spool settings saved</label>";
          } else {
            document.getElementById("message").innerHTML = "<label class=\"errormsg\">Error: " + this.responseText + "</label>";
          }
          setTimeout(function () {
            document.getElementById("message").innerHTML = "<label class=\"msg\">&nbsp;</label>";
          }, 5000);
        }
      };
      xhr.open("POST", "/spooldata", true);
      xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      xhr.send(postdata);
    }

    function loadFilaments(vendor) {
      // ... (loadFilaments function remains unchanged) ...
      if (vendor == "") { vendor = "Generic"; }
      const objDb = JSON.parse(jsonDb);
      filaments = "";
      for (let i in objDb.result.list) {
        if (objDb.result.list[i].base.brand == vendor) {
          filaments += "<option value=\"" + objDb.result.list[i].base.id + "\">" + objDb.result.list[i].base.name + "</option>";
        }
      }
    }

    function change_types() {
      // ... (change_types function remains unchanged) ...
      var selectedValue = document.getElementById("materialBrand").value;
      loadFilaments(selectedValue);
      document.getElementById("types").innerHTML = "<select name=\"materialType\" id=\"materialType\">" + filaments + "</select>";
    }

    function loadCache() {
      // ... (loadCache function remains unchanged) ...
      if (localStorage.getItem("materialBrand") == null) {
        loadFilaments("");
        document.getElementById("types").innerHTML = "<select name=\"materialType\" id=\"materialType\">" + filaments + "</select>";
        return;
      }
      document.getElementById("materialBrand").value = localStorage.getItem("materialBrand");
      var selectedValue = document.getElementById("materialBrand").value;
      change_types(selectedValue);
      document.getElementById("materialType").value = localStorage.getItem("materialType");
      document.getElementById("materialWeight").value = localStorage.getItem("materialWeight");
      document.getElementById("materialColor").value = localStorage.getItem("materialColor");
    }

    function clearCache() {
      // ... (clearCache function remains unchanged) ...
      localStorage.clear();
      document.getElementById("materialBrand").value = "Generic";
      change_types();
      document.getElementById("materialType").value = "00001";
      document.getElementById("materialWeight").value = "1 KG";
      document.getElementById("materialColor").value = "#0000FF";
      sendData(); // Note: sendData might not be desired on clearCache, but this is original behavior.
    }

    function openConf() { /* ... (remains unchanged) ... */ }
    function saveConf() { /* ... (remains unchanged) ... */ }
    function loadVendors() { /* ... (remains unchanged) ... */ }
    function loadDb() { /* ... (remains unchanged, ensure jsonDb is globally populated) ... */ }
    function loadParam() { /* ... (remains unchanged) ... */ }
    function loadUpdate() { /* ... (remains unchanged) ... */ }
    function loadDbUpdate() { /* ... (remains unchanged) ... */ }
    function formatBytes(bytes) { /* ... (remains unchanged) ... */ }
    function FileSelected(e) { /* ... (remains unchanged) ... */ }
    function DbFileSelected(e) { /* ... (remains unchanged) ... */ }
    function uploadFile() { /* ... (remains unchanged) ... */ }
    function uploadDb() { /* ... (remains unchanged) ... */ }
    function downloadDb() { /* ... (remains unchanged) ... */ }
    const uploadDbFromPrinter = async () => { /* ... (remains unchanged) ... */ };
    function redirect() { /* ... (remains unchanged) ... */ }

    // --- Start of Read/Write Mode JavaScript Functions ---
    function updateModeLabel() {
      var messageArea = document.getElementById("message");
      if (!modeToggle || !modeLabel || !loadCardDataBtn) return; // Elements not ready

      if (modeToggle.checked) { // Checked means "Read Mode"
        modeLabel.textContent = "Read Mode";
        currentSpoolerMode = "read";
        loadCardDataBtn.style.display = "inline-block"; // Show button
        if(messageArea) messageArea.innerHTML = "<label class='msg'>Switched to Read Mode. Present card then click 'Load Read Data'.</label>";
      } else { // Unchecked means "Write Mode"
        modeLabel.textContent = "Write Mode";
        currentSpoolerMode = "write";
        loadCardDataBtn.style.display = "none";   // Hide button
        if(messageArea) messageArea.innerHTML = "<label class='msg'>Switched to Write Mode.</label>";
      }
       setTimeout(function () {
         if(messageArea) messageArea.innerHTML = "<label class='msg'>&nbsp;</label>";
       }, 5000);
    }

    function handleModeChange() {
      updateModeLabel();
      // Send mode to ESP32
      var xhr = new XMLHttpRequest();
      xhr.open("POST", "/setmode", true);
      xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      // No onreadystatechange needed here unless we want specific feedback for mode change POST
      xhr.send("mode=" + currentSpoolerMode);
      console.log("Mode set request sent to ESP32: " + currentSpoolerMode);
    }

    function fetchAndDisplayCardData() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/getCardData", true);
      xhr.onreadystatechange = function () {
        if (this.readyState == 4) {
          var messageArea = document.getElementById("message");
          if (!messageArea) { console.error("Message area not found"); return; }

          if (this.status == 200) {
            try {
              var data = JSON.parse(this.responseText);
              if (data.success) {
                messageArea.innerHTML = "<label class='msg'>Card data loaded into form.</label>";

                if (data.materialColor) {
                  document.getElementById("materialColor").value = data.materialColor;
                }
                if (data.materialWeight) {
                  document.getElementById("materialWeight").value = data.materialWeight;
                }

                if (data.materialType) { // data.materialType is the filament ID
                  var filamentIDToSelect = data.materialType;
                  var brandFoundForID = false;
                  var brandToSelect = "Generic"; // Default

                  if (jsonDb) { 
                    const objDb = JSON.parse(jsonDb); // Ensure jsonDb is parsed
                    for (let i in objDb.result.list) {
                      if (objDb.result.list[i].base.id == filamentIDToSelect) {
                        brandToSelect = objDb.result.list[i].base.brand;
                        brandFoundForID = true;
                        break;
                      }
                    }
                  } else {
                     console.error("jsonDb is not loaded. Cannot determine brand.");
                     messageArea.innerHTML = "<label class='errormsg'>Material DB not loaded. Cannot determine brand.</label>";
                  }
                  
                  document.getElementById("materialBrand").value = brandToSelect;
                  change_types(); // This re-populates materialType dropdown based on brand
                  document.getElementById("materialType").value = filamentIDToSelect; 

                  if (!brandFoundForID && jsonDb) { // Only show this specific message if DB was available
                    messageArea.innerHTML = "<label class='msg'>Card data loaded. Note: Brand for type " + filamentIDToSelect + " not in local DB; selected " + brandToSelect + ".</label>";
                  }
                }
              } else {
                messageArea.innerHTML = "<label class='errormsg'>Could not retrieve card data: " + (data.message || "N/A") + "</label>";
              }
            } catch (e) {
              messageArea.innerHTML = "<label class='errormsg'>Error parsing card data response.</label>";
              console.error("Error parsing card data:", e, this.responseText);
            }
          } else {
            messageArea.innerHTML = "<label class='errormsg'>Failed to fetch card data (HTTP " + this.status + ")</label>";
          }
          setTimeout(function () {
            if(messageArea) messageArea.innerHTML = "<label class='msg'>&nbsp;</label>";
          }, 7000);
        }
      };
      xhr.send();
    }
    // --- End of Read/Write Mode JavaScript Functions ---

    window.addEventListener("DOMContentLoaded", function () {
      loadDb(); // Original: loads material DB, vendors, and cache

      // --- Read/Write Mode Initializations ---
      modeToggle = document.getElementById("modeToggle");
      modeLabel = document.getElementById("modeLabel");
      loadCardDataBtn = document.getElementById("loadCardDataBtn");

      if(modeToggle && modeLabel && loadCardDataBtn) { // Ensure all elements are found
          modeToggle.addEventListener('change', handleModeChange);
          loadCardDataBtn.addEventListener('click', fetchAndDisplayCardData);
          updateModeLabel(); // Set initial label and button visibility based on default toggle state
      } else {
        console.error("Mode toggle elements not found on DOMContentLoaded!");
      }
      // --- End Read/Write Mode Initializations ---
    });
  </script>
</head>
<body>
  <center>
    <div class="main">
      <table>
        <tr> <!-- Icons row -->
          <td>
            <br>
            <label style="display: block;text-align: left;">
              <svg xmlns="http://www.w3.org/2000/svg" height="32px" viewBox="0 0 24 24" width="32px"
                onClick="loadParam();">
                <path d="M0 0h24v24H0V0z" fill="none" />
                <path
                  d="M11 7h2v2h-2zm0 4h2v6h-2zm1-9C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm0 18c-4.41 0-8-3.59-8-8s3.59-8 8-8 8 3.59 8 8-3.59 8-8 8z" />
              </svg>
            </label>
          </td>
          <td>
            <br>
            <label style="display: block;text-align: right;">
              <svg xmlns="http://www.w3.org/2000/svg" height="32px" viewBox="0 0 24 24" width="32px"
                onClick="openConf();">
                <path d="M0,0h24v24H0V0z" fill="none" />
                <path
                  d="M19.14,12.94c0.04-0.3,0.06-0.61,0.06-0.94c0-0.32-0.02-0.64-0.07-0.94l2.03-1.58c0.18-0.14,0.23-0.41,0.12-0.61 l-1.92-3.32c-0.12-0.22-0.37-0.29-0.59-0.22l-2.39,0.96c-0.5-0.38-1.03-0.7-1.62-0.94L14.4,2.81c-0.04-0.24-0.24-0.41-0.48-0.41 h-3.84c-0.24,0-0.43,0.17-0.47,0.41L9.25,5.35C8.66,5.59,8.12,5.92,7.63,6.29L5.24,5.33c-0.22-0.08-0.47,0-0.59,0.22L2.74,8.87 C2.62,9.08,2.66,9.34,2.86,9.48l2.03,1.58C4.84,11.36,4.8,11.69,4.8,12s0.02,0.64,0.07,0.94l-2.03,1.58 c-0.18,0.14-0.23,0.41-0.12,0.61l1.92,3.32c0.12,0.22,0.37,0.29,0.59,0.22l2.39-0.96c0.5,0.38,1.03,0.7,1.62,0.94l0.36,2.54 c0.05,0.24,0.24,0.41,0.48,0.41h3.84c0.24,0,0.44-0.17,0.47-0.41l0.36-2.54c0.59-0.24,1.13-0.56,1.62-0.94l2.39,0.96 c0.22,0.08,0.47,0,0.59-0.22l1.92-3.32c0.12-0.22,0.07-0.47-0.12-0.61L19.14,12.94z M12,15.6c-1.98,0-3.6-1.62-3.6-3.6 s1.62-3.6,3.6-3.6s3.6,1.62,3.6,3.6S13.98,15.6,12,15.6z" />
              </svg>
            </label>
          </td>
        </tr>
        <tr>
          <th colspan="2" style="background-color:#E0E0E0">
            <center>&nbsp;</center> <!-- Spacer -->
          </th>
        </tr>
        <tr>
          <td><b>Filament Brand:</b></td>
          <td>
            <div class="brands" id="brands">
              <select name="materialBrand" id="materialBrand" onchange="change_types()"></select> <!-- onchange added -->
            </div>
          </td>
        </tr>
        <tr>
          <td><b>Filament Type:</b></td>
          <td>
            <div class="types" id="types">
              <select name="materialType" id="materialType"></select>
            </div>
          </td>
        </tr>
        <tr>
          <td><b>Spool Size:</b></td>
          <td>
            <select name="materialWeight" id="materialWeight">
              <option value="1 KG">1 KG</option>
              <option value="750 G">750 G</option>
              <option value="600 G">600 G</option>
              <option value="500 G">500 G</option>
              <option value="250 G">250 G</option>
            </select>
          </td>
        </tr>
        <tr>
          <td><b>Filament Color:</b></td>
          <td>
            <input name="materialColor" id="materialColor" type="color" value="#0000FF" list="commonColors" />
            <datalist id="commonColors">
              <option value="#1200F6" /><option value="#3894E1" /><option value="#FEFF01" /><option value="#F8D531" /><option value="#F38E24" /><option value="#52D048" /><option value="#00FEBE" /><option value="#B700F3" /><option value="#EE301A" /><option value="#FA5959" /><option value="#FFFFFF" /><option value="#D8D8D8" /><option value="#4C4C4C" /><option value="#782543" /><option value="#000000" />
            </datalist>
          </td>
        </tr>
        <!-- Read/Write Mode Toggle Row -->
        <tr>
          <td><b>Mode:</b></td>
          <td>
            <label class="switch">
              <input type="checkbox" id="modeToggle">
              <span class="slider round"></span>
            </label>
            <span id="modeLabel">Write Mode</span>
          </td>
        </tr>
        <!-- Load Read Data Button Row -->
        <tr>
            <td></td> <!-- Empty cell for alignment -->
            <td>
                <button type="button" class="btn" id="loadCardDataBtn" style="display:none; padding: 10px 15px; font-size: 14px; margin-left:0;">Load Read Data</button>
            </td>
        </tr>
      </table>
      <br>
      <div id="message"><label class="msg">&nbsp;</label></div>
      <br>
      <input type="button" class="btn" value="Reset" onClick="clearCache()">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="btn" value="Apply" onClick="sendData()">
    </div>
  </center>
  
  <!-- Dialogs (cDialog, pDialog, uDialog, dDialog) remain unchanged -->
  <dialog id="cDialog"> /* ... content ... */ </dialog>
  <dialog id="pDialog"> /* ... content ... */ </dialog>
  <dialog id="uDialog"> /* ... content ... */ </dialog>
  <dialog id="dDialog"> /* ... content ... */ </dialog>

  <script>
    // This ensures dialog consts are defined before they might be used in other functions if those functions were moved up.
    const dialog = document.getElementById("cDialog");
    const dialogp = document.getElementById("pDialog");
    const dialogu = document.getElementById("uDialog");
    const dialogd = document.getElementById("dDialog");
    // The rest of the JavaScript, including existing functions and new ones, goes here.
    // Make sure all variables used across functions are declared at an appropriate scope.
  </script>
</body>
</html>
)==";
