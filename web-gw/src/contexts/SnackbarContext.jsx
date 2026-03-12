import { createContext } from 'preact';
import { useState, useContext, useCallback } from 'preact/hooks';
import Snackbar from '../components/snackbar.jsx';

// CreateContext parameters can be a default value
const SnackbarContext = createContext({
    showSnackbar: () => {}
});

export const useSnackbar = () => useContext(SnackbarContext);

export const SnackbarProvider = ({ children }) => {
    const [snackbarData, setSnackbarData] = useState({
        message: '',
        type: 'info', // 'success', 'error', 'info'
        isVisible: false
    });

    const showSnackbar = useCallback((message, type = 'info') => {
        setSnackbarData({ message, type, isVisible: true });
        
        // Auto-close after 3 seconds
        setTimeout(() => {
            setSnackbarData(prev => ({ ...prev, isVisible: false }));
        }, 3000);
    }, []);

    const hideSnackbar = useCallback(() => {
        setSnackbarData(prev => ({ ...prev, isVisible: false }));
    }, []);

    return (
        <SnackbarContext.Provider value={{ showSnackbar }}>
            {children}
            <Snackbar 
                message={snackbarData.message} 
                type={snackbarData.type} 
                isVisible={snackbarData.isVisible}
                onClose={hideSnackbar}
            />
        </SnackbarContext.Provider>
    );
};
