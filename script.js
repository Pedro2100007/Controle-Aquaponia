// Configurações do ThingSpeak
const channelID = '2840207';
const readAPIKey = '5UWNQD21RD2A7QHG';
const writeAPIKey = '9NG6QLIN8UXLE2AH';

// Elementos da interface
const elements = {
  temperature: document.getElementById('temperature'),
  level: document.getElementById('level'),
  statusBomba: document.getElementById('statusBomba'),
  statusAquecedor: document.getElementById('statusAquecedor'),
  modoAutomatico: document.getElementById('modoAutomatico'),
  bombaOn: document.getElementById('bombaOn'),
  bombaOff: document.getElementById('bombaOff'),
  resistenciaOn: document.getElementById('resistenciaOn'),
  resistenciaOff: document.getElementById('resistenciaOff')
};

// Função para atualizar a interface
function updateUI(data) {
  console.log("Dados recebidos para atualização:", data);
  
  // Temperatura (field1)
  if (data.field1 !== undefined && data.field1 !== null) {
    elements.temperature.textContent = parseFloat(data.field1).toFixed(1);
  }
  
  // Nível (field2)
  if (data.field2 !== undefined && data.field2 !== null) {
    elements.level.textContent = parseFloat(data.field2).toFixed(1);
  }
  
  // Bomba (field3)
  if (data.field3 !== undefined && data.field3 !== null) {
    const bombaState = parseInt(data.field3);
    elements.statusBomba.textContent = bombaState ? 'Bomba Ligada' : 'Bomba Desligada';
    elements.statusBomba.style.color = bombaState ? 'green' : 'red';
  }
  
  // Aquecedor (field4)
  if (data.field4 !== undefined && data.field4 !== null) {
    const resistenciaState = parseInt(data.field4);
    elements.statusAquecedor.textContent = resistenciaState ? 'Aquecedor Ligado' : 'Aquecedor Desligado';
    elements.statusAquecedor.style.color = resistenciaState ? 'green' : 'red';
  }
  
  // Modo Automático (field5)
  if (data.field5 !== undefined && data.field5 !== null) {
    const modoAutoState = parseInt(data.field5);
    elements.modoAutomatico.textContent = modoAutoState ? 'Ligado' : 'Desligado';
  }
}

// Função para buscar dados do ThingSpeak
async function fetchData() {
  try {
    const response = await fetch(`https://api.thingspeak.com/channels/${channelID}/feeds/last.json?api_key=${readAPIKey}`);
    
    if (!response.ok) {
      throw new Error(`Erro HTTP: ${response.status}`);
    }
    
    const data = await response.json();
    console.log("Dados completos da API:", data);
    
    if (!data || Object.keys(data).length === 0) {
      throw new Error("Dados vazios recebidos da API");
    }
    
    updateUI(data);
    return data;
    
  } catch (error) {
    console.error("Erro ao buscar dados:", error);
    return null;
  }
}

// Função para atualizar um campo no ThingSpeak
async function updateField(field, value) {
  try {
    const url = `https://api.thingspeak.com/update?api_key=${writeAPIKey}&${field}=${value}`;
    const response = await fetch(url, { method: 'POST' });
    
    if (!response.ok) {
      throw new Error(`Erro HTTP: ${response.status}`);
    }
    
    const result = await response.text();
    console.log(`Campo ${field} atualizado. Resposta:`, result);
    
    // Força atualização imediata
    await fetchData();
    
  } catch (error) {
    console.error(`Erro ao atualizar ${field}:`, error);
  }
}

// Configuração dos event listeners
function setupEventListeners() {
  // Controle manual da bomba
  elements.bombaOn?.addEventListener('click', () => {
    const modoAuto = elements.modoAutomatico.textContent === 'Ligado';
    if (!modoAuto) updateField('field3', 1);
    else alert('Desative o modo automático primeiro');
  });

  elements.bombaOff?.addEventListener('click', () => {
    const modoAuto = elements.modoAutomatico.textContent === 'Ligado';
    if (!modoAuto) updateField('field3', 0);
    else alert('Desative o modo automático primeiro');
  });

  // Controle manual do aquecedor
  elements.resistenciaOn?.addEventListener('click', () => {
    const modoAuto = elements.modoAutomatico.textContent === 'Ligado';
    if (!modoAuto) updateField('field4', 1);
    else alert('Desative o modo automático primeiro');
  });

  elements.resistenciaOff?.addEventListener('click', () => {
    const modoAuto = elements.modoAutomatico.textContent === 'Ligado';
    if (!modoAuto) updateField('field4', 0);
    else alert('Desative o modo automático primeiro');
  });

  // Controle do modo automático
  elements.modoAutomatico?.addEventListener('click', () => {
    const currentState = elements.modoAutomatico.textContent === 'Ligado';
    updateField('field5', currentState ? 0 : 1);
  });
}

// Inicialização
document.addEventListener('DOMContentLoaded', () => {
  console.log("Página carregada. Iniciando configuração...");
  
  // Verificação dos elementos
  for (const [key, element] of Object.entries(elements)) {
    if (!element) console.error(`Elemento não encontrado: ${key}`);
  }
  
  setupEventListeners();
  fetchData();
  
  // Atualização periódica
  setInterval(fetchData, 5000);
  console.log("Configuração completa. Monitorando dados...");
});

//------------------------------------------------------- implementado para envio de parâmetros
// Captura o input do tempoLiga
const tempoLigaInput = document.getElementById('tempoLiga');
// Função de envio do valor ao ThingSpeak
function enviarTempoLiga() {
  const tempoL = parseInt(tempoLigaInput.value);
  // Validação básica
  //if (isNaN(tempoL) || tempoL < 0) {
  //  alert("Digite um número válido (0 ou maior)");
  //  return;
  //}
  updateField(7, tempoL)
}
// Dispara o evento (Enter ou clicar fora)
tempoLigaInput.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') enviarTempoLiga();
});
tempoLigaInput.addEventListener('blur', enviarTempoLiga);

//-----
// Captura o input do tempoDesliga
const tempoDesligaInput = document.getElementById('tempoDesliga');
// Função de envio do valor ao ThingSpeak
function enviarTempoDesliga() {
  const tempoD = parseInt(tempoDesligaInput.value);
  // Validação básica
  //if (isNaN(tempoD) || tempoD < 0) {
  //  alert("Digite um número válido (0 ou maior)");
  //  return;
  //}
  updateField(8, tempoD)
}
// Dispara o evento (Enter ou clicar fora)
tempoDesligaInput.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') enviarTempoDesliga();
});
tempoDesligaInput.addEventListener('blur', enviarTempoDesliga);

//-----
// Captura o input do temperaturaAlvo
const temperaturaAlvoInput = document.getElementById('temperaturaAlvo');
// Função de envio do valor ao ThingSpeak
function enviartemperaturaAlvo() {
  const tempAl = parseInt(temperaturaAlvoInput.value);
  // Validação básica
  //if (isNaN(tempAl) || tempAl < 0) {
  //  alert("Digite um número válido (0 ou maior)");
  //  return;
  //}
  updateField(6, tempAl)
}
// Dispara o evento (Enter ou clicar fora)
temperaturaAlvoInput.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') enviartemperaturaAlvo();
});
temperaturaAlvoInput.addEventListener('blur', enviartemperaturaAlvo);
