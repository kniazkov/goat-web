var xmlHttpObject = null;
var server = "http://127.0.0.1:8001"; //window.location.protocol + '//' + window.location.host;

var getXmlHttp = function()
{
    if (xmlHttpObject)
        return xmlHttpObject;

    try {
        xmlHttpObject = new ActiveXObject('Msxml2.XMLHTTP');
    }
    catch (e0) {
        try {
            xmlHttpObject = new ActiveXObject('Microsoft.XMLHTTP');
        }
        catch (e1) {
            xmlHttpObject = false;
        }
    }
    if (!xmlHttpObject && typeof XMLHttpRequest!='undefined')
        xmlHttpObject = new XMLHttpRequest();
    return xmlHttpObject;
};

var errConnectionMsg = null;
var errConnectionsCount = 0;

var showErrConnectionMsg = function() {
    if (errConnectionsCount < 10) {
        errConnectionsCount++;
    }
    else if (errConnectionMsg == null) {
        errConnectionMsg = document.createElement("div");
        errConnectionMsg.className = "noConnection";
        errConnectionMsg.innerHTML = "No connection to the server";
        document.body.appendChild(errConnectionMsg);
    }
};

var hideErrConnectionMsgIfExists = function() {
    errConnectionsCount = 0;
    if (errConnectionMsg != null) {
        document.body.removeChild(errConnectionMsg);
        errConnectionMsg = null;
    }
};

var sendRequest = function(request, callback) {
    var req = getXmlHttp()
    req.open('GET', server + '?request=' + encodeURIComponent(request), true);
    req.onreadystatechange = function() {
        if (req.readyState == 4) {
            if(req.status == 200) {
                hideErrConnectionMsgIfExists();
                callback(req.responseText);
            }
        }
    };
    req.onerror = showErrConnectionMsg;
    req.send(null);
};

var sendRequestWithData = function(request, data, callback)
{
    var req = getXmlHttp();
    req.open('GET', server + '?request=' + encodeURIComponent(request) + '&data=' + encodeURIComponent(JSON.stringify(data)), true);
    req.onreadystatechange = function() {
        if (req.readyState == 4) {
            if(req.status == 200) {
                hideErrConnectionMsgIfExists();
                callback(req.responseText);
            }
        }
    };
    req.onerror = showErrConnectionMsg;
    req.send(null);
};

var sendRequestWithDataAndFile = function(request, data, file, callback)
{
    var req = getXmlHttp();
    var form = new FormData();
    form.append("request", request);
    form.append("data", JSON.stringify(data));
    form.append("file", file);

    req.open('POST', server, true);
    req.onreadystatechange = function() {
        if (req.readyState == 4) {
            if(req.status == 200) {
                hideErrConnectionMsgIfExists();
                if (callback)
                    callback(req.responseText);
            }
        }
    };
    req.onerror = showErrConnectionMsg;
    req.send(form);
};

var addEvent = function(object, type, callback)
{
    if (typeof(object) == "string")
        object = document.getElementById(object);
    if (object == null || typeof(object) == "undefined")
        return;

    if (object.addEventListener)
        object.addEventListener(type, callback, false);
    else if (object.attachEvent)
        object.attachEvent("on" + type, callback);
    else
        object["on" + type] = callback;
};

var urlParameters = null;

var getUrlParameters = function() {
    if (urlParameters)
        return urlParameters;

    urlParameters = { };
    var query = window.location.search.substring(1);
    var vars = query.split('&');
    var i;
    for (i = 0; i < vars.length; i++) {
        var pair = vars[i].split('=');
        var key = decodeURIComponent(pair[0]);
        var value = decodeURIComponent(pair[1]);
        urlParameters[key] = value;
    }
    return urlParameters;
};

var formatString = function(format) {
    var result = format;
    for (var i = 1; i < arguments.length; i++)
    {
        result = result.replace('{' + (i - 1) + '}', arguments[i]);
    }
    return result;
};

var escapeHtml = function(unsafe)
{
    return unsafe
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
};

var uIdCounter = 0;

var generateUid = function() {
    ++uIdCounter;
    return "#" + uIdCounter;
};

/*
var generateUid = function() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
};
*/
