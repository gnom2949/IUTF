document.addEventListener('DOMContentLoaded', () => {
  console.log("G78E doc initialized");

  const body = document.body;
  const prismLight = document.getElementById('prism-theme-light');
  const prismDark = document.getElementById('prism-theme-dark');
  const sysD = window.matchMedia('(prefers-color-scheme: dark)');

  let themeBtns;

  if (!document.querySelector('.theme-switch')) {
    const switchDiv = document.createElement('div');
    switchDiv.className = 'theme-switch';
    switchDiv.innerHTML = `
      <button class="theme-btn active" data-theme="light">Light</button>
      <button class="theme-btn" data-theme="dark">Dark</button>
      <button class="theme-btn" data-theme="system">Auto</button>
    `;
    body.prepend(switchDiv);

    themeBtns = document.querySelectorAll('.theme-btn');
    console.log("theme switch successfully loaded");
  }

  themeBtns = document.querySelectorAll('.theme-btn');

  function setify(theme) {
    const isDark = (theme === 'dark') || (theme === 'system' && sysD.matches);

    body.classList.toggle('dark-theme', isDark);
    body.classList.toggle('light-theme', !isDark);

    if (prismLight && prismDark) {
      prismLight.disabled = isDark;
      prismDark.disabled = !isDark;
    }

    themeBtns.forEach(btn => {
      btn.classList.toggle('active', btn.dataset.theme === theme);
    });

    body.setAttribute('data-theme', theme);
    localStorage.setItem('itdocg78', theme);

    console.log(`Theme set to: ${theme} (dark: ${isDark})`); // для отладки
  }

  themeBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      setify(btn.dataset.theme);
    });
  });

  sysD.addEventListener('change', () => {
    if (localStorage.getItem('itdocg78') === 'system') {
      setify('system');
    }
  });

  const savedTheme = localStorage.getItem('itdocg78') || 'light';
  setify(savedTheme);

  document.querySelectorAll('pre').forEach(block => {
    if (block.querySelector('.copy-btn, .g78doc-code-copy-btn')) return;

    const button = document.createElement('button');
    button.textContent = 'Copy';
    button.className = 'g78doc-code-copy-btn copy-btn';
    button.title = 'Copy to clipboard';

    button.onclick = async () => {
      try {
        const code = block.querySelector('code') || block;
        await navigator.clipboard.writeText(code.innerText);

        button.textContent = '✓';
        button.style.backgroundColor = '#2ec27e';
        button.classList.add('copied');

        setTimeout(() => {
          button.textContent = 'Copy';
          button.style.backgroundColor = '';
          button.classList.remove('copied');
        }, 2000);
      } catch (err) {
        console.error('Copy failed:', err);
        button.textContent = '✗';
        button.style.backgroundColor = '#e01b24';
        button.classList.add('error');

        setTimeout(() => {
          button.textContent = 'Copy';
          button.style.backgroundColor = '';
          button.classList.remove('error');
        }, 2000);
      }
    };

    block.style.position = 'relative';
    block.appendChild(button);
  });

  document.querySelectorAll('pre').forEach(block => {
    const code = block.querySelector('code');
    if (code && code.className) {
      const match = code.className.match(/language-(\w+)/);
      if (match) {
        block.setAttribute('data-language', match[1].toUpperCase());
      }
    }
  });

  document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', (e) => {
      e.preventDefault();
      const target = document.querySelector(anchor.getAttribute('href'));
      if (target) {
        target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      }
    });
  });

  console.log("G78E doc fully loaded");
});
