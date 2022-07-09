var container = document.getElementById('container');
var pages = document.querySelectorAll('.page');
// var slip = Slip(container, 'y').webapp(pages);
Slip(document.getElementById('container'), 'y').webapp();
Slip(document.getElementById('container'), 'y').webapp().end(function() {
    if (this.page === 5) location.reload();
});