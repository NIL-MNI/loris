<html>
<head>
<title>Loris Form Builder</title>
<style>
{literal}
.selected {
    background: #099;
}
{/literal}
</style>

<script type="application/ecmascript" src="js/BlobBuilder.min.js"></script>
<script type="application/ecmascript" src="js/FileSaver.min.js"></script>
</head>
<body>
<div id="message" class="error">&nbsp;</div>
<div id="elements">
    <h2>Load Instrument (optional)</h2>
    <div>
        <input type="file" id="instfile" /><input type="button" id="load" value="Load Instrument" />
    </div>
    <h2>Create Instrument</h2>
    <div>
        Question Type:
        <dl style="margin-left: 25px">
            <dt>Data entry</dt>
            <dd>
            <button id="textbox" class="option">Textbox</button>
            <button id="textarea" class="option">Textarea</button>
            <button id="dropdown" class="option">Dropdown</button>
            <button id="multiselect" class="option">Multiselect</button>
            <button id="date" class="option">Date</button>
            </dd>
            <dt>Information</dt>
            <dd>
            <button id="header" class="option">Header</button>
            <button id="label" class="option">Label</button>
            <button id="scored" class="option">Scored Column</button>
            <dt>Formatting</dt>
            <dd>
            <button id="line" class="option">Blank Line</button>
            <button id="page" class="option">Page Break</button>
            </dd>
        </dl>
        <div>

            <div>
        Question Name: <input type="text" id="questionName" />
            </div>
            <div>
        Question Text: <input type="text" id="questionText" />
            </div>
            <div id="dropdownoptions" class="options">
                Dropdown Option: <input type="text" id="newSelectOption">
                    <input type="button" value="Add option" onClick="addDropdownOption();" />
                    <input type="button" value="Reset" onClick="clearDropdownOption()" />
                Preview:
                <select id="selectOptions">
                </select>
            </div>
            <div id="multiselectoptions" class="options">
                Option: <input type="text" id="newmultiSelectOption">
                    <input type="button" value="Add option" onClick="addDropdownOption('multi');" />
                    <input type="button" value="Reset" onClick="clearDropdownOption('multi')" />
                Preview:
                <select id="multiselectOptions" multiple>
                </select>
            </div>
        </div>

        <input type="button" value="Add Row" onClick="addQuestion();" />
        <table id="workspace" border="1">
        <tr>
            <th>Database Name (DB)</th>
            <th>Type</th>
            <th>Question Display (Front End)</th>
            <th>Options</th>
        </tr>
        </table>
    </div>
    <h2>Save Instrument</h2>
    <div>

        <form name="saveform" id="saveform">
        Filename: <input type="text" id="filename" value="" />
        Instrument Name: <input type="text" id="longname" value="" />
        <input type="button" onclick="validate()" value="Validate" />
        <input type="submit" value="Save" />
        </form>
    </div>
</div>
</body>
</html>
