
// === public/script.js ===
// 1) عرض آخر نتيجة
function fetchLastResult() {
  fetch('/last-result')
    .then(r => r.json())
    .then(data => {
      const statusDiv  = document.getElementById('status');
      const detailsDiv = document.getElementById('details');

      if (data.status === 'granted') {
        statusDiv.textContent = '✅ Access Granted';
        statusDiv.style.color = 'green';
        detailsDiv.innerHTML = `
          <p><strong>ID:</strong> ${data.id}</p>
          <p><strong>Name:</strong> ${data.name}</p>
          <p><strong>Phone:</strong> ${data.phone}</p>
          <p><strong>Travel Date:</strong> ${data.travel_date}</p>
        `;
      } else if (data.status === 'denied') {
        statusDiv.textContent = '❌ Access Denied';
        statusDiv.style.color = 'red';
        detailsDiv.innerHTML = `
          <p>${data.reason}</p>
          ${data.name ? `<p><strong>Name:</strong> ${data.name}</p>` : ''}
          ${data.id   ? `<p><strong>ID:</strong> ${data.id}</p>` : ''}
        `;
      } else {
        statusDiv.textContent = '⚠️ No scan yet';
        statusDiv.style.color = '#555';
        detailsDiv.innerHTML = '';
      }
    });
}

// 2) إضافة عميل جديد
document.getElementById('addClientForm').addEventListener('submit', e => {
  e.preventDefault();
  const name        = document.getElementById('newName').value.trim();
  const phone       = document.getElementById('newPhone').value.trim();
  const travel_date = document.getElementById('newDate').value;

  fetch('/add-client', {
    method : 'POST',
    headers: { 'Content-Type': 'application/json' },
    body   : JSON.stringify({ name, phone, travel_date })
  })
  .then(r => r.json())
  .then(data => {
    const result = document.getElementById('addResult');
    if (data.success) {
      result.style.color = 'green';
      result.innerHTML = `
        ✅ Client added successfully.<br/>
        <strong>Client ID:</strong> ${data.id}<br/>
        <img src="${data.qr}" alt="QR Code" style="margin-top:10px;width:300px;height:300px;object-fit:contain;" /><br/>
        <a href="${data.qr}" download="qr-${data.id}.png">⬇️ Download QR</a>
      `;
      document.getElementById('addClientForm').reset();
    } else {
      result.textContent = '❌ ' + data.message;
      result.style.color = 'red';
    }
  });
});

// 3) تحميل جدول بكل العملاء
function loadClients() {
  fetch('/all-clients')
    .then(r => r.json())
    .then(rows => {
      const tbody = document.querySelector('#clientsTable tbody');
      tbody.innerHTML = '';
      rows.forEach(c => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
          <td>${c.name}</td>
          <td>${c.phone}</td>
          <td>${c.travel_date}</td>
          <td>${c.id}</td>
          <td><a href="/generate-qr/${c.id}" target="_blank" download="qr-${c.id}.png">⬇️ Download</a></td>`;
        tbody.appendChild(tr);
      });
    });
}

// loadClients , fetchLastResult(); every 5 seconds
setInterval(() => {
  fetchLastResult();
}, 1000);

setInterval(() => {
  loadClients();
}, 1000);