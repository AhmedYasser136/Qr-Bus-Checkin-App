
function fetchLastResult() {
  fetch('/last-result')
    .then(res => res.json())
    .then(data => {
      const statusDiv = document.getElementById('status');
      const detailsDiv = document.getElementById('details');
      if (data.status === 'granted') {
        statusDiv.textContent = '✅ Access Granted';
        statusDiv.style.color = 'green';
        detailsDiv.innerHTML = `
          <p><strong>ID:</strong> ${data.data}</p>
          <p><strong>Name:</strong> ${data.name}</p>
          <p><strong>Phone:</strong> ${data.phone}</p>
          <p><strong>Travel Date:</strong> ${data.travel_date}</p>
        `;
      } else if (data.status === 'denied') {
        statusDiv.textContent = '❌ Access Denied';
        statusDiv.style.color = 'red';
        detailsDiv.innerHTML = `
          <p>${data.reason}</p>
          ${data.name ? `<p><strong>Name:</strong> ${data.name}</p>` : ""}
        `;
      } else {
        statusDiv.textContent = '⚠️ No scan yet';
        statusDiv.style.color = '#555';
        detailsDiv.innerHTML = '';
      }
    });
}

document.getElementById('addClientForm').addEventListener('submit', (e) => {
  e.preventDefault();
  const name = document.getElementById('newName').value.trim();
  const phone = document.getElementById('newPhone').value.trim();
  const travel_date = document.getElementById('newDate').value;

  fetch('/add-client', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ name, phone, travel_date })
  })
    .then(res => res.json())
    .then(data => {
      const result = document.getElementById('addResult');
      if (data.success) {
        result.style.color = "green";
        result.innerHTML = `
          ✅ Client added successfully.<br/>
          <strong>Client ID:</strong> ${data.id}<br/>
          <img src="${data.qr}" alt="QR Code" style="margin-top:10px; width:200px;" /><br/>
          <a href="${data.qr}" download="qr-${data.id}.png">⬇️ Download QR</a>
        `;
        document.getElementById('addClientForm').reset();
      } else {
        result.textContent = "❌ " + data.message;
        result.style.color = "red";
      }
    });
});

function loadClients() {
  fetch('/all-clients')
    .then(res => res.json())
    .then(data => {
      const tbody = document.querySelector('#clientsTable tbody');
      tbody.innerHTML = '';
      data.forEach(client => {
        const row = document.createElement('tr');
        row.innerHTML = `
          <td>${client.name}</td>
          <td>${client.phone}</td>
          <td>${client.travel_date}</td>
          <td>${client.id}</td>
        `;
        tbody.appendChild(row);
      });
    });
}