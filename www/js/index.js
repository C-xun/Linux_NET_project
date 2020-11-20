$(function () {	
    $("#reset").click(function () 
    {
    $("form")[0].reset();
    $("#IP").focus();
    });
})
function getXMLHttpRequest()
{
    var xmlhttp = null;
    if(window.XMLHttpRequest)
    {
        xmlhttp = new XMLHttpRequest();
    }
    else
    {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    return xmlhttp;
}

function my_route(arg)
{
    var xmlhttp = getXMLHttpRequest();

    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            alert(url);
            document.getElementById("txt").innerHTML = xmlhttp.responseText;
            document.getElementById("txt").value = xmlhttp.responseText;
        }
    }

    url = "/cgi-bin/deal.cgi?";

    if(arg == 0)
    {
        url +="hello_config";
    }
    else if(arg == 1)
    {
        url +="filter_ip";
        url +="ip_";
        url +=document.getElementById("IP").value;
    }
    else if(arg == 2)
    {
        url +="filter_mac";
        url +="mac_";
        url +=document.getElementById("MAC").value;
    }
    else if(arg == 3)
    {
        url +="route_add";
        url +=":"
        url +=document.getElementById("route").value;
    }
    else if(arg == 4)
    {
        url +="hello_route";
    }
    else if(arg == 5)
    {
        url +="hello_arp";
    }

    xmlhttp.open("GET", url, true);
    xmlhttp.send();
}