$(document).ready(function() {
  function time() {
    var now = new Date();
    var hours = now.getHours()
    var minutes = now.getMinutes();
    var seconds = now.getSeconds();
    
    if (hours > 12)
      hours = hours - 12;
    if (hours == 0)
      hours = 12
            
    if (minutes < 10)
      minutes = "0" + minutes
    if (seconds < 10)
      seconds = "0" + seconds
      
    return {
      hours: hours.toString(),
      minutes: minutes.toString(),
      seconds: seconds.toString(),
      ordinal: (now.getHours() > 11) ? 'PM' : 'AM'
    }
  }
  
  var $hours = $(".hours");
  var $minutes = $(".minutes");
  var $seconds = $(".seconds");
  // 
  var now = time();
  $hours.html(now.hours + "<i>" + now.ordinal + "</i>");
  $minutes.html(now.minutes);
  $seconds.html(now.seconds);
  
  $hours.flippanel()
  $minutes.flippanel()
  $seconds.flippanel()
  
  setInterval(function() {
    now = time(); 
    $(".hours").flippanel(now.hours + "<i>" + now.ordinal + "</i>");
    $(".minutes").flippanel(now.minutes);
    $(".seconds").flippanel(now.seconds);
    
  }, 1000)
});