import * as React from 'react';
import { DataGrid } from '@mui/x-data-grid';
import { tableColumns, tableRows } from './table_data';

export default function DataTable() {
  return (
    <div style={{ height: '675%', width: '100%' }}>
      <DataGrid
        rows={tableRows}
        columns={tableColumns}
        pageSize={5}
        rowsPerPageOptions={[5]}
        checkboxSelection
      />
    </div>
  );
}
