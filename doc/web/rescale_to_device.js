function println(text) {
    console.log(text);
    if (textoutelem) {
      textoutelem.value += text + "\n";
      textoutelem.scrollTop = textoutelem.scrollHeight; // focus on bottom
    }
};
function fixResolution(name) {
    var devicePixelRatio = window.devicePixelRatio || 1;
    fixResolutionImpl(name, devicePixelRatio, devicePixelRatio);
}
function fixResolutionImpl(name, devicePixelRatioX, devicePixelRatioY) {
    var target = document.getElementById(name);
    var ecr = target.getBoundingClientRect();
    var nwidth = ecr.width/devicePixelRatioX;
    var nheight = ecr.height/devicePixelRatioY;
    println(`devicePixelRatio ${devicePixelRatioX} x ${devicePixelRatioY}`);
    println(`element ${name}: ${ecr.width} x ${ecr.height} -> ${nwidth} x ${nheight}`);
    target.setAttribute("style",`width:${nwidth}px`);
    target.setAttribute("style",`height:${nheight}px`);
}
