fetch("../../../components/Navbar/Navbar.html")
	.then((res) => res.text())
	.then((html) => {
		document.getElementById("navbar").innerHTML = html;
	});

fetch("../../components/Footer/Footer.html")
	.then((res) => res.text())
	.then((html) => {
		document.getElementById("footer").innerHTML = html;
	});
