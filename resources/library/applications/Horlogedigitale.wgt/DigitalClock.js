function animate() {
    var theCanvas = document.getElementById('myCanvas');


    if (theCanvas.width != document.body.clientWidth) {
        theCanvas.width = document.body.clientWidth;
    }

    if (theCanvas.height != document.body.clientHeight - 30) {
        theCanvas.height = document.body.clientHeight - 30;
    }

    var context = theCanvas.getContext('2d');

// clear
    context.clearRect(0, 0, theCanvas.width, theCanvas.height);


// draw
    context.fillStyle = '#00F';
    context.textBaseline = 'middle';

    var currentDateTime = new Date();

    var hours = currentDateTime.getHours();
    if (hours < 10) {
        hours = "0" + hours;
    }

    var minutes = currentDateTime.getMinutes();
    if (minutes < 10) {
        minutes = "0" + minutes;
    }

    var seconds = currentDateTime.getSeconds();
    if (seconds < 10) {
        seconds = "0" + seconds;
    }

    var timeString = hours + ':' + minutes + ':' + seconds;

    context.font = 'bold ' + theCanvas.width / 5 + 'px sans-serif';

    var dim = context.measureText(timeString);

    var y = (theCanvas.height - 30) / 2;
    var x = (theCanvas.width - dim.width) / 2;


//  							alert('x '+x+' y '+y);

    context.fillText(timeString, x, y);
}

setInterval("animate()", 1000);
