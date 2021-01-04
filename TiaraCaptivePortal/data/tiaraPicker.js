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

var colorPicker = new iro.ColorPicker(".colorPicker", {
  width: 480,
  margin: 24,
  color: "rgb(255,0,255)",
  borderWidth: 3,
  borderColor: "#fff",
  handleRadius: 20,
});

colorPicker.on(["color:init","color:change"], function (color) {
  var x = new XMLHttpRequest();
  x.onload = function() {
    var status = x.status;
    var data = x.responseText;
  }
  x.open("POST","/wheel",true);
  x.setRequestHeader('Content-type','application/x-www-form-urlencoded');
  x.send('rgb='+color.hexString);
});