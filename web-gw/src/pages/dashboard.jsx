import { useState, useEffect } from 'preact/hooks';
import './dashboard.css';

export default function Dashboard() {
    // mock metric data
    const [metrics] = useState({
        total: 120,
        active: 98,
        avgTemp: 22.5,
        uptime: 99.9
    });

    // generate a small array of temperature readings for trend
    const [tempTrend] = useState([20, 22, 25, 23, 21, 24, 26, 27, 24, 22]);

    const [status] = useState({
        active: 98,
        available: 22
    });

    return (
        <div className="dashboard fade-in">
            <div className="kpi-row">
                <div className="kpi-card total">
                    <div className="kpi-icon">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"></path><polyline points="3.29 7 12 12 20.71 7"></polyline><line x1="12" y1="22" x2="12" y2="12"></line></svg>
                    </div>
                    <div className="kpi-content">
                        <h3>Total Sensores</h3>
                        <p className="kpi-value">{metrics.total}</p>
                    </div>
                </div>
                <div className="kpi-card active">
                    <div className="kpi-icon">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path><polyline points="22 4 12 14.01 9 11.01"></polyline></svg>
                    </div>
                    <div className="kpi-content">
                        <h3>Sensores Activos</h3>
                        <p className="kpi-value">{metrics.active}</p>
                    </div>
                </div>
                <div className="kpi-card temp">
                    <div className="kpi-icon">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"></path></svg>
                    </div>
                    <div className="kpi-content">
                        <h3>Temp. Promedio</h3>
                        <p className="kpi-value">{metrics.avgTemp}&deg;C</p>
                    </div>
                </div>
                <div className="kpi-card uptime">
                    <div className="kpi-icon">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="22 12 18 12 15 21 9 3 6 12 2 12"></polyline></svg>
                    </div>
                    <div className="kpi-content">
                        <h3>Disponibilidad</h3>
                        <p className="kpi-value">{metrics.uptime}%</p>
                    </div>
                </div>
            </div>

            <div className="charts-row">
                <div className="chart temperature-trend">
                    <div className="chart-header">
                        <h4>Tendencia de Temperatura</h4>
                        <span className="chart-subtitle">Lecturas de las últimas 24 horas (&deg;C)</span>
                    </div>
                    <div className="chart-container-inner">
                        <div className="y-axis">
                            <span>28°</span>
                            <span>24°</span>
                            <span>20°</span>
                            <span>16°</span>
                        </div>
                        <div className="chart-wrapper">
                            <svg viewBox="0 -5 100 60" preserveAspectRatio="none">
                                <defs>
                                    <linearGradient id="trendGradient" x1="0" y1="0" x2="0" y2="1">
                                        <stop offset="0%" stopColor="#3b82f6" stopOpacity="0.4" />
                                        <stop offset="100%" stopColor="#3b82f6" stopOpacity="0" />
                                    </linearGradient>
                                </defs>
                                {/* Grid lines */}
                                <line x1="0" y1="0" x2="100" y2="0" className="chart-grid-line" />
                                <line x1="0" y1="16.6" x2="100" y2="16.6" className="chart-grid-line" />
                                <line x1="0" y1="33.3" x2="100" y2="33.3" className="chart-grid-line" />
                                <line x1="0" y1="50" x2="100" y2="50" className="chart-grid-line" />

                                <path
                                    d={`M 0 50 ${tempTrend.map((v, i) => `L ${i * (100 / (tempTrend.length - 1))} ${50 - (v - 15) * 4}`).join(' ')} L 100 50 Z`}
                                    fill="url(#trendGradient)"
                                />
                                <polyline
                                    fill="none"
                                    stroke="#3b82f6"
                                    strokeWidth="2"
                                    strokeLinejoin="round"
                                    strokeLinecap="round"
                                    points={tempTrend
                                        .map((v, i) => `${i * (100 / (tempTrend.length - 1))},${50 - (v - 15) * 4}`)
                                        .join(' ')}
                                />
                            </svg>
                            <div className="x-axis">
                                <span>00h</span>
                                <span>06h</span>
                                <span>12h</span>
                                <span>18h</span>
                                <span>Ahora</span>
                            </div>
                        </div>
                    </div>
                </div>
                <div className="chart sensor-status">
                    <div className="chart-header">
                        <h4>Estado de Sensores</h4>
                        <span className="chart-subtitle">Resumen operativo</span>
                    </div>
                    <div className="donut-container">
                        <svg viewBox="0 0 36 36" className="donut">
                            <circle
                                className="donut-ring"
                                cx="18"
                                cy="18"
                                r="15.5"
                                fill="transparent"
                                strokeWidth="3"
                            />
                            <circle
                                className="donut-segment"
                                cx="18"
                                cy="18"
                                r="15.5"
                                fill="transparent"
                                strokeWidth="3"
                                strokeDasharray={`${(status.active / (status.active + status.available)) * 100} 100`}
                                strokeDashoffset="25"
                            />
                        </svg>
                        <div className="donut-label">
                            <span className="main-val">{status.active}</span>
                            <span className="sub-val">Operativos</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}
