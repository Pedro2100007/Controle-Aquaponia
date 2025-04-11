// Configurações do ThingSpeak
const channelID = '2840207';
const readAPIKey = '5UWNQD21RD2A7QHG';
const writeAPIKey = '9NG6QLIN8UXLE2AH';
const temperatureField = 'field1';
const levelField = 'field2';
const bombaField = 'field3';
const resistenciaField = 'field4';
const modoAutomaticoField = 'field5';
const temperaturaAlvoField = 'field6'; 
const tempoLigaField = 'field7';
const tempoDesligaField = 'field8';

// Elementos da seção de controle manual
const temperatureElement = document.getElementById('temperature');
const levelElement = document.getElementById('level');
const bombaOnButton = document.getElementById('bombaOn');
const bombaOffButton = document.getElementById('bombaOff');
const resistenciaOnButton = document.getElementById('resistenciaOn');
const resistenciaOffButton = document.getElementById('resistenciaOff');
const botaoModoAuto = document.getElementById('modoAutomatico'); // Adicionado

// Variáveis para armazenar os últimos valores válidos do thingspeak
let modoAutomatico = 0;
let lastValidTemperature = '--';
let lastValidLevel = '--';

// Função para buscar dados do ThingSpeak em tempo real
function fetchData() {
    fetch(`https://api.thingspeak.com/channels/${channelID}/feeds/last.json?api_key=${readAPIKey}`)
        .then(response => {
            if (!response.ok) {
                throw new Error('Erro na requisição: ' + response.statusText);
            }
            return response.json();
        })
        .then(data => {
            console.log('Dados recebidos:', data);

            // Atualiza a temperatura se o campo não estiver vazio
            if (data[temperatureField] && data[temperatureField].trim() !== '') {
                lastValidTemperature = data[temperatureField];
            }
            temperatureElement.textContent = lastValidTemperature;

            // Atualiza o nível se o campo não estiver vazio
            if (data[levelField] && data[levelField].trim() !== '') {
                lastValidLevel = data[levelField];
            }
            levelElement.textContent = lastValidLevel;

            // Atualiza o status da bomba
            if (data[bombaField] == 1) {
                statusBomba.textContent = 'Bomba Ligada';
                statusBomba.style.color = 'green';
            } else {
                statusBomba.textContent = 'Bomba Desligada';
                statusBomba.style.color = 'red';
            }
            
            // Atualiza o status do aquecedor
            if (data[resistenciaField] == 1) {
                statusAquecedor.textContent = 'Aquecedor Ligado';
                statusAquecedor.style.color = 'green';
            } else {
                statusAquecedor.textContent = 'Aquecedor Desligado';
                statusAquecedor.style.color = 'red';
            }

            // Atualiza o estado do botão modo automático
            const modoAutoValue = data[modoAutomaticoField];
            if (modoAutoValue != null) { // Verifica se o campo existe
                modoAutomatico = parseInt(modoAutoValue);
                botaoModoAuto.textContent = modoAutomatico === 1 ? 'Ligado' : 'Desligado';
            }

        })
        .catch(error => {
            console.error('Erro ao buscar dados:', error);
            temperatureElement.textContent = lastValidTemperature;
            levelElement.textContent = lastValidLevel;
        });
}

// Atualiza os dados em tempo real a cada 5 segundos
if (temperatureElement && levelElement) {
    setInterval(fetchData, 5000);
    fetchData();
}

// Função para atualizar um campo no ThingSpeak
function updateField(field, value) {
    const url = `https://api.thingspeak.com/update?api_key=${writeAPIKey}&${field}=${value}`;
    console.log(`Enviando requisição para: ${url}`);

    fetch(url, {
        method: 'POST'
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Erro na requisição: ' + response.statusText);
        }
        return response.text();
    })
    .then(data => {
        console.log(`Campo ${field} atualizado com sucesso. Resposta: ${data}`);
        // Após atualizar com sucesso, força uma nova busca dos dados
        fetchData();
    })
    .catch(error => {
        console.error(`Erro ao atualizar campo ${field}:`, error);
    });
}

// Event listeners para os botões do controle manual
if (bombaOnButton && bombaOffButton && resistenciaOnButton && resistenciaOffButton) {
    bombaOnButton.addEventListener('click', () => {
        if (modoAutomatico === 0) {
            console.log('Ligando bomba...');
            updateField(bombaField, 1);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    bombaOffButton.addEventListener('click', () => {
        if (modoAutomatico === 0) {
            console.log('Desligando bomba...');
            updateField(bombaField, 0);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    resistenciaOnButton.addEventListener('click', () => {
        if (modoAutomatico === 0) {
            console.log('Ligando resistência...');
            updateField(resistenciaField, 1);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });

    resistenciaOffButton.addEventListener('click', () => {
        if (modoAutomatico === 0) {
            console.log('Desligando resistência...');
            updateField(resistenciaField, 0);
        } else {
            alert('Modo automático está ligado. Desligue o modo automático para controlar manualmente.');
        }
    });
}

// Função para controle automático
function toggleModoAutomatico() {
    const novoEstado = modoAutomatico === 0 ? 1 : 0;
    
    // Atualiza imediatamente a interface para melhor experiência do usuário
    modoAutomatico = novoEstado;
    botaoModoAuto.textContent = novoEstado === 1 ? 'Ligado' : 'Desligado';
    
    // Envia a atualização para o ThingSpeak
    updateField(modoAutomaticoField, novoEstado);
}

// Inicializa a página buscando os dados imediatamente
document.addEventListener('DOMContentLoaded', function() {
    fetchData();
});