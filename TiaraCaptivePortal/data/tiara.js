window.onload = function() {
  tiara_status();
  setInterval(tiara_status,1000);
};

function tiara_status() {
  console.log('status update\n');
  var d = new Date();
  var m = d.getMilliseconds().toString();
  var x = new XMLHttpRequest();
  x.onreadystatechange = function() {
    if(this.readyState == 4 && this.status == 200) {
      var o = JSON.parse(this.responseText);
      document.getElementById('status').innerHTML = 'Current Pattern: ' + o.pattern;
    }
  };
  x.open('GET','/status',true);
  x.send();
}
