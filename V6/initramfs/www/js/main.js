$(function(){
  init();
  show_os_info();
  show_network_info();
});

function init() {
  $("#home").addClass("active");
}

function show_os_info() {
  ajaxRequest("cgi-bin/os-info.sh", "div#os-info");
  ajaxRequest("cgi-bin/uptime.sh", "div#uptime");
}

function show_network_info() {
  ajaxRequest("cgi-bin/network-info.sh", "textarea#console");
}

var ajaxRequest = function (url, copy_to_element) {
    request = $.ajax({ url : url, type : "post", contentType : "text/html", data : null });

    //request.done(function (response, textStatus, jqXHR){ console.log(response); });
    
    request.fail(function (jqXHR, textStatus, errorThrown){ console.log("error from " + url) });
    
    request.success(function(data){
            $(copy_to_element).text(data);
            console.log("success from " + url)
        }
    );
}

$('[href="#network-info"]').click(function() {
  ajaxRequest("cgi-bin/network-info.sh", "textarea#console");    
});

$('[href="#cpu-info"]').click(function() {
  ajaxRequest("cgi-bin/cpu-info.sh", "textarea#console");
});

$('[href="#mem-info"]').click(function() {
  ajaxRequest("cgi-bin/mem-info.sh", "textarea#console");
});

$('[href="#proc-info"]').click(function() {
  ajaxRequest("cgi-bin/proc-info.sh", "textarea#console");
});
$('[href="#shutdown"]').click(function() {
  ajaxRequest("cgi-bin/shutdown.sh", "textarea#console");
});
$('[href="#reboot"]').click(function() {
  ajaxRequest("cgi-bin/reboot.sh", "textarea#console");
});

$('a#ls-tmp').click(function() {
  ajaxRequest("cgi-bin/ls-tmp.sh", "textarea#console");
});

$('a#delete-tmp').click(function() {
  ajaxRequest("cgi-bin/rm-tmp.sh", "textarea#console");
f});
