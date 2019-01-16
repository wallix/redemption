function square(x, y, w, h, color) {
  var canvas = document.getElementById("canvas");
  var context = canvas.getContext('2d');
  var colors = { 0:"SeaGreen",
                 1:"Blue", 2:"Crimson", 3:"Coral", 4:"Cyan",
                 5:"Chartreuse", 6:"Fuchsia ", 7:"Indigo", 8:"LightPink",
                 9:"Gray", 10:"Orange", 11:"Yellow", 12:"Black" };
  
  context.fillStyle = colors[color];
  context.fillRect(
    x * canvas.width/12, canvas.height - (y+1) * canvas.height/5,
    canvas.width/12, canvas.height/5);
};

window.onload = function() {
  _run_main();
  document.getElementById("canvas").addEventListener('click', function() {_run_main();}); 
};

//square(0, 0, 12, 5, 'GoldenRod')

