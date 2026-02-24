fetch("../../../components/Navbar/Navbar.html")
	.then((res) => res.text())
	.then((html) => {
		document.getElementById("navbar").innerHTML = html;

		const normalizedPath = window.location.pathname.replace(/\/$/, "");
		const navLinks = document.querySelectorAll("#navbar .nav_links a");

		navLinks.forEach((link) => {
			const href = link.getAttribute("href");
			if (!href) return;

			const normalizedHref = href.replace(/\/$/, "");
			const isHome = normalizedHref === "" || normalizedHref === "/";

			if (
				(isHome && (normalizedPath === "" || normalizedPath === "/")) ||
				normalizedPath === normalizedHref
			) {
				link.classList.add("active");
			}
		});
	});

fetch("../../components/Footer/Footer.html")
	.then((res) => res.text())
	.then((html) => {
		document.getElementById("footer").innerHTML = html;
	});
