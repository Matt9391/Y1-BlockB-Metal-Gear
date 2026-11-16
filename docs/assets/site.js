/* Front-website behaviour, shared by the hand-written pages and the guides.
 * Vanilla JS, no dependencies; every page reads fine without it.
 *
 *   - Copy on code blocks (emitted by docs/build_guides.py)
 *   - the guide list as a disclosure on narrow screens
 *   - "On this page" following the section being read
 *   - search over the guides and the reference, in a dialog
 *     (index: assets/search-index.js, written by build_guides.py)
 */
(function () {
  "use strict";

  var root = document.documentElement;
  var base = root.getAttribute("data-root") || "";

  // --- copy ------------------------------------------------------------------
  function copyText(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      return navigator.clipboard.writeText(text);
    }
    // Fallback for insecure contexts (opening the files over file://).
    return new Promise(function (resolve, reject) {
      var ta = document.createElement("textarea");
      ta.value = text;
      ta.setAttribute("readonly", "");
      ta.style.position = "fixed";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      var ok = false;
      try { ok = document.execCommand("copy"); } catch (e) { ok = false; }
      document.body.removeChild(ta);
      ok ? resolve() : reject();
    });
  }

  document.addEventListener("click", function (e) {
    var btn = e.target.closest(".copy");
    if (!btn) return;
    var code = btn.closest(".excerpt").querySelector("code");
    copyText(code.innerText).then(
      function () { flash(btn, "Copied", true); },
      function () { flash(btn, "Press Ctrl+C", false); }
    );
  });

  function flash(btn, label, ok) {
    btn.textContent = label;
    btn.classList.toggle("copied", ok);
    clearTimeout(btn._t);
    btn._t = setTimeout(function () { btn.textContent = "Copy"; btn.classList.remove("copied"); }, 1600);
  }

  // --- guide list on narrow screens -----------------------------------------
  function initSide() {
    var btn = document.querySelector(".side-toggle");
    var nav = document.getElementById("side-nav");
    if (!btn || !nav) return;
    nav.hidden = true;
    btn.addEventListener("click", function () {
      var open = btn.getAttribute("aria-expanded") !== "true";
      btn.setAttribute("aria-expanded", String(open));
      nav.hidden = !open;
    });
  }

  // --- on this page ------------------------------------------------------------
  function initToc() {
    var links = Array.prototype.slice.call(document.querySelectorAll(".toc a"));
    if (!links.length || !("IntersectionObserver" in window)) return;
    var byId = {};
    links.forEach(function (a) { byId[decodeURIComponent(a.hash.slice(1))] = a; });
    var headings = links.map(function (a) { return document.getElementById(decodeURIComponent(a.hash.slice(1))); }).filter(Boolean);
    var visible = new Set();
    function mark() {
      var current = headings.filter(function (h) { return visible.has(h); })[0];
      if (!current) {
        for (var j = headings.length - 1; j >= 0; j--) {
          if (headings[j].getBoundingClientRect().top < 140) { current = headings[j]; break; }
        }
      }
      links.forEach(function (a) { a.removeAttribute("aria-current"); });
      if (current && byId[current.id]) byId[current.id].setAttribute("aria-current", "true");
    }
    var io = new IntersectionObserver(function (entries) {
      entries.forEach(function (en) { en.isIntersecting ? visible.add(en.target) : visible.delete(en.target); });
      mark();
    }, { rootMargin: "-80px 0px -60% 0px" });
    headings.forEach(function (h) { io.observe(h); });
  }

  // --- search ------------------------------------------------------------------
  function initSearch() {
    var dialog = document.getElementById("search");
    var openBtn = document.querySelector(".search-open");
    if (!dialog || !openBtn) return;
    var index = window.BUZ_SEARCH;
    if (!index || !index.length || !dialog.showModal) {
      // No index (build_guides.py writes it) or no <dialog> support: no search.
      openBtn.hidden = true;
      return;
    }
    var input = dialog.querySelector("input");
    var list = dialog.querySelector(".search-results");
    var items = index.map(function (e) {
      return { t: e.t, k: e.k, w: e.w || "", u: e.u,
               hay: (e.t + " " + (e.w || "")).toLowerCase(),
               last: e.t.toLowerCase().split("::").pop() };
    });
    var results = [];
    var active = -1;

    function score(it, q, words) {
      for (var i = 0; i < words.length; i++) if (it.hay.indexOf(words[i]) < 0) return 0;
      var t = it.t.toLowerCase();
      var id = q.replace(/\s+/g, "_");   // "draw sprite" finds draw_sprite
      var s = 1;
      if (it.last === id || t === q) s += 100;
      else if (it.last.indexOf(id) === 0) s += 60;
      else if (t.indexOf(id) >= 0 || t.indexOf(q) >= 0) s += 30;
      if (it.k === "Guide") s += 8;
      if (it.k === "Class" || it.k === "Struct") s += 5;
      return s - t.length / 100;
    }

    function render() {
      list.innerHTML = "";
      if (!input.value.trim()) return;
      if (!results.length) {
        var none = document.createElement("li");
        none.className = "r-empty";
        none.textContent = "Nothing found. Try a function name, such as draw_sprite.";
        list.appendChild(none);
        return;
      }
      results.forEach(function (it, i) {
        var li = document.createElement("li");
        li.id = "sr-" + i;
        li.setAttribute("role", "option");
        li.setAttribute("aria-selected", String(i === active));
        var a = document.createElement("a");
        a.href = base + it.u;
        a.tabIndex = -1;
        if (it.k !== "Guide" && it.k !== "Section") a.className = "sym";
        var t = document.createElement("span"); t.className = "r-title"; t.textContent = it.t;
        var k = document.createElement("span"); k.className = "r-kind"; k.textContent = it.k;
        a.appendChild(t); a.appendChild(k);
        if (it.w) { var w = document.createElement("span"); w.className = "r-where"; w.textContent = it.w; a.appendChild(w); }
        li.appendChild(a);
        list.appendChild(li);
      });
    }

    function setActive(i) {
      active = i;
      Array.prototype.forEach.call(list.querySelectorAll("[role=option]"), function (li, j) {
        li.setAttribute("aria-selected", String(j === i));
        if (j === i) li.scrollIntoView({ block: "nearest" });
      });
      if (i >= 0) input.setAttribute("aria-activedescendant", "sr-" + i);
      else input.removeAttribute("aria-activedescendant");
    }

    function open() {
      if (dialog.open) return;
      dialog.showModal();
      input.focus();
      input.select();
    }

    input.addEventListener("input", function () {
      var q = input.value.trim().toLowerCase();
      results = !q ? [] : items
        .map(function (it) { return { it: it, s: score(it, q, q.split(/\s+/)) }; })
        .filter(function (r) { return r.s > 0; })
        .sort(function (a, b) { return b.s - a.s; })
        .slice(0, 12)
        .map(function (r) { return r.it; });
      active = results.length ? 0 : -1;
      render();
      setActive(active);
    });

    input.addEventListener("keydown", function (e) {
      if (e.key === "ArrowDown" && results.length) { e.preventDefault(); setActive((active + 1) % results.length); }
      else if (e.key === "ArrowUp" && results.length) { e.preventDefault(); setActive((active - 1 + results.length) % results.length); }
      else if (e.key === "Enter" && active >= 0) { e.preventDefault(); window.location.href = base + results[active].u; }
    });

    openBtn.addEventListener("click", open);
    // a click on the backdrop closes the dialog
    dialog.addEventListener("click", function (e) { if (e.target === dialog) dialog.close(); });

    // "/" or Ctrl+K opens search from anywhere on the page
    document.addEventListener("keydown", function (e) {
      var el = document.activeElement;
      var typing = el && (/^(INPUT|TEXTAREA|SELECT)$/.test(el.tagName) || el.isContentEditable);
      if ((e.key === "/" && !typing) || (e.key.toLowerCase() === "k" && (e.ctrlKey || e.metaKey))) {
        e.preventDefault();
        open();
      }
    });
  }

  function init() {
    root.classList.add("js");
    initSide();
    initToc();
    initSearch();
  }

  if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", init);
  else init();
})();
