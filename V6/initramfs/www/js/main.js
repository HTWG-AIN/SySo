
var running_getstates = 0;

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
  running_getstates = 0;
  ajaxRequest("cgi-bin/network-info.sh", "textarea#console");
}

var ajaxRequest = function (url, copy_to_element) {
    request = $.ajax({ url : url, type : "post", contentType : "text/html", data : null });

    //request.done(function (response, textStatus, jqXHR){ console.log(response); });
    
    request.fail(function (jqXHR, textStatus, errorThrown){ console.log("error from " + url) });
    
    request.success(function(data){
            $(copy_to_element).text(data);
        }
    );
}

$('[href="#network-info"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/network-info.sh", "textarea#console");    
  }, 400);
});

$('[href="#cpu-info"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/cpu-info.sh", "textarea#console");
  },400);
});

$('[href="#mem-info"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/mem-info.sh", "textarea#console");
  },400);
});

$('[href="#proc-info"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/proc-info.sh", "textarea#console");
  },400);
});
$('[href="#shutdown"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/shutdown.sh", "textarea#console");
  },400);
});
$('[href="#reboot"]').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/reboot.sh", "textarea#console");
  },400);
});

$('a#ls-tmp').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/ls-tmp.sh", "textarea#console");
  },400);
});

$('a#delete-tmp').click(function() {
  running_getstates = 0;
  setTimeout(function() {
    ajaxRequest("cgi-bin/rm-tmp.sh", "textarea#console");
  },400);
});

$('[href="#toggle-led"]').click(function() {
    ajaxRequest("cgi-bin/gpio-toggle-led.sh", "textarea#console");
});

$('[href="#gpio"]').click(function() {
  var getstates =  function() {
    ajaxRequest("cgi-bin/gpio-getstates.sh", "textarea#console");
    if (running_getstates) {
      setTimeout(getstates, 199);
    }
  }
  running_getstates = 1;
  var gpio_interval = setTimeout(getstates, 199);
});

