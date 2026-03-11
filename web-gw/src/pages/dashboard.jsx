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
        <div className="dashboard">
            <div className="kpi-row">
                <div className="kpi-card">
                    <h3>Total Sensors</h3>
                    <p className="kpi-value">{metrics.total}</p>
                </div>
                <div className="kpi-card">
                    <h3>Active Sensors</h3>
                    <p className="kpi-value">{metrics.active}</p>
                </div>
                <div className="kpi-card">
                    <h3>Avg. Temperature</h3>
                    <p className="kpi-value">{metrics.avgTemp}&deg;C</p>
                </div>
                <div className="kpi-card">
                    <h3>System Uptime</h3>
                    <p className="kpi-value">{metrics.uptime}%</p>
                </div>
            </div>

            <div className="charts-row">
                <div className="chart temperature-trend">
                    <h4>Temperature Trends</h4>
                    <svg viewBox="0 0 100 50" preserveAspectRatio="none">
                        <polyline
                            fill="none"
                            stroke="#3b82f6"
                            strokeWidth="2"
                            points={tempTrend
                                .map((v, i) => `${i * (100 / (tempTrend.length - 1))},${50 - v}`)
                                .join(' ')}
                        />
                    </svg>
                </div>
                <div className="chart sensor-status">
                    <h4>Sensor Status</h4>
                    <div className="donut-container">
                        <svg viewBox="0 0 36 36" className="donut">
                            <circle
                                className="donut-ring"
                                cx="18"
                                cy="18"
                                r="15"
                                fill="transparent"
                                strokeWidth="6"
                            />
                            <circle
                                className="donut-segment"
                                cx="18"
                                cy="18"
                                r="15"
                                fill="transparent"
                                strokeWidth="6"
                                strokeDasharray={`${status.active},${status.active + status.available}`}
                                strokeDashoffset="25"
                            />
                        </svg>
                        <div className="donut-label">
                            {status.active} / {status.active + status.available}
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}
